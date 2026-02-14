#include "histogramd.h"
#include "histogram.h"
#include "ia.h"

HistogramD::HistogramD(double beg, double end, const QVector<double>& vals) : values(vals)
{
	keys.reserve(vals.size());
	for (int i = 0; i < vals.size(); ++i)
	{
		keys.append(beg + i * (end - beg) / (vals.size() - 1));
	}
}

HistogramD::HistogramD(const Ia& ia, const QVector<double>& vals) : HistogramD(ia[0], ia[1], vals)
{

}

HistogramD::HistogramD(const Histogram& h) : HistogramD(h.Range(),
	[](const QVector<int>& v) {
		QVector<double> d;
		d.reserve(v.size());
		for (int x : v) d.append(x);
		return d;
	}(h.GetHistogram()))
{

}

HistogramD::HistogramD(const QVector<double>& keys, const QVector<double>& vals) : keys(keys), values(vals)
{

}

HistogramD::HistogramD(const ScalarField2& sf, int size) : HistogramD(sf.GetHistogram(size))
{

}

HistogramD::HistogramD(const ScalarField2& sf, int size, double a, double b) : HistogramD(Histogram(size, a, b,
	[](const ScalarField2& sf, double a, double b) {
		QVector<double> v;
		for (double h : sf)
			if (h >= a && h <= b)
				v.append(h);
		return v;
	}(sf, a, b)))
{

}

HistogramD HistogramD::operator*(double f) const
{
	QVector<double> v = values;

	for (int i = 0; i < Size(); ++i)
	{
		v[i] *= f;
	}

	return HistogramD(keys, v);
}

HistogramD HistogramD::operator/(double f) const
{
	QVector<double> v = values;

	for (int i = 0; i < Size(); ++i)
	{
		v[i] /= f;
	}

	return HistogramD(keys, v);
}

// Return the closest index to v with bigger value than v
// keys sould be sorted
int HistogramD::UpperKey(double v) const
{
	int n = Size();
	
	if (v <= keys[0]) return 0;

	// TODO: faire une dichotomie
	for (int i = 1; i < n; ++i)
	{
		if (v <= keys[i])
		{
			return i;
		}
	}
	return n;
}

// Return the closest index to v with lower value than v
// keys sould be sorted
int HistogramD::LowerKey(double v) const
{
	int n = Size();

	if (v <= keys[0]) return 0;

	// TODO: faire une dichotomie
	for (int i = 1; i < n; ++i)
	{
		if (v < keys[i])
		{
			return i - 1;
		}
	}
	return n;
}

// Return the closest index to v
// keys sould be sorted
int HistogramD::ClosestKey(double v) const
{
	int n = Size();

	if (v <= keys[0]) return 0;

	// TODO: faire une dichotomie
	for (int i = 1; i < n; ++i)
	{
		if (v < keys[i])
		{
			if (keys[i] - v < v - keys[i - 1]) return i;
			return i - 1;
		}
	}
	return n;
}

// return the largest bin of the histo
int HistogramD::MaxValue() const
{
	int ind = 0;
	double val = values[0];
	for (int i = 1; i < values.size(); ++i)
	{
		if (values[i] > val)
		{
			val = values[i];
			ind = i;
		}
	}
	return ind;
}

// return the lowest bin of the histo
int HistogramD::MinValue() const
{
	int ind = 0;
	double val = values[0];
	for (int i = 1; i < values.size(); ++i)
	{
		if (values[i] < val)
		{
			val = values[i];
			ind = i;
		}
	}
	return ind;
}

double HistogramD::GetSum() const
{
	double val = 0;
	for (int i = 0; i < values.size(); ++i)
	{
		val += values[i];
	}
	return val;
}

HistogramD HistogramD::ReversedHistogram(bool reverseKeys) const
{
	QVector<double> vs = values;
	QVector<double> ks = keys;

	std::reverse(vs.begin(), vs.end());
	if (reverseKeys)
		std::reverse(ks.begin(), ks.end());

	return HistogramD(ks, vs);
}

HistogramD HistogramD::CumulativeHistogram() const
{
	QVector<double> vals;
	vals.reserve(values.size());

	double c = 0;
	for (int i = 0; i < values.size(); ++i)
	{
		c += values[i];
		vals.append(c);
	}

	return HistogramD(keys, vals);
}

HistogramD HistogramD::NormalizedHistogram(double min, double max) const
{
	QVector<double> vals;
	vals.reserve(values.size());
	double M = values[MaxValue()];

	for (int i = 0; i < values.size(); ++i)
	{
		double t = values[i] / M;
		double v = t * max + (1 - t) * min;
		vals.append(v);
	}

	return HistogramD(keys, vals);
}

HistogramD HistogramD::Ceil() const
{
	QVector<double> vals;
	vals.reserve(values.size());

	for (int i = 0; i < values.size(); ++i)
	{
		vals.append(Math::Ceil(values[i]));
	}

	return HistogramD(keys, vals);
}

HistogramD HistogramD::Floor() const
{
	QVector<double> vals;
	vals.reserve(values.size());

	for (int i = 0; i < values.size(); ++i)
	{
		vals.append(Math::Floor(values[i]));
	}

	return HistogramD(keys, vals);
}

/*
	Construct an histogram with n values, trying to keep the same distribution as the current histogram
	The distribution of the current histogram must be in equally distributed bins.
*/
Histogram HistogramD::ToHistogram(int n) const
{
	QVector<double> v;
	v.reserve(n);
	HistogramD h = *this; // copy for draining

	double drain = GetSum() / n;
	int currentInd = 0;

	for (int j = 0; j < n; ++j)
	{
		double d = drain; // remaining drain
		double m = -1;    // maximum drain in a bin
		int i = -1;

		// We remove "drain" from the histogram, and we add the height corresponding to the "most drained" bin.
		while (d > 0)
		{
			double v = h(currentInd);
			// first case, the bin is big enough to be drained
			if (v - d >= 0)
			{
				if (d > m)
				{
					m = d;
					i = currentInd;
				}
				h(currentInd) -= d;
				break;
			}

			// second case, the bin is set to 0 because it is full drained
			if (v > m)
			{
				m = v;
				i = currentInd;
			}
			h(currentInd) = 0;
			d -= v;

			if (currentInd == Size() - 1)
			{
				// if this is the last assignation, we can have a small error due to double but we don't want an error
				if (j == n - 1)
				{
					i = currentInd;
					break;
				}

				qDebug() << "Not enough value in the histogram during the assignation" << j << "/" << n << ".This should not happen.";
				exit(1);
			}

			currentInd++;
		}

		v.append(h.GetKey(i));
	}

	return Histogram(Size(), GetKey(0), GetKey(Size() - 1), v);

}

#include <QtWidgets/QGraphicsRectItem>
void HistogramD::Draw(QGraphicsScene& scene, const GenericPalette& palette, double w, double h) const
{
	double max = GetValue(MaxValue());
	double ratioValPix = max == 0 ? 1 : h / max;
	double binSize = w / Size();
	for (int i = 0; i < Size(); ++i)
	{
		double t = i / (Size() - 1.);
		QColor col = palette.GetColor(t).GetQt();
		QGraphicsRectItem* item = scene.addRect(i * binSize, 0, binSize, ratioValPix * Math::Max(GetValue(i), 0.1));
		item->setBrush(QBrush(col));
		item->setPen(QPen(Qt::black, binSize / 10.));
	}
}
