#pragma once

#include "scalarfield.h"
#include "palette.h"

#include <QtWidgets/QGraphicsScene>

class Ia;
class Histogram;

class HistogramD
{
protected:
	QVector<double> keys;
	QVector<double> values;

public:
	HistogramD() {}
	HistogramD(double, double, const QVector<double>&);
	HistogramD(const Ia&, const QVector<double>&);
	HistogramD(const Histogram&);
	HistogramD(const QVector<double>&, const QVector<double>&);
	HistogramD(const ScalarField2&, int);
	HistogramD(const ScalarField2&, int, double, double);

	double operator()(int) const;
	double& operator()(int);
	HistogramD operator*(double) const;
	HistogramD operator/(double) const;

	int Size() const;
	double GetValue(int) const;
	double GetKey(int) const;
	void Remove(int); // Do not use, or be aware that it changes every previous indices

	int MaxValue() const;
	double GetSum() const;

	HistogramD Ceil() const;
	HistogramD Floor() const;

	Histogram ToHistogram(int) const;
	// Ne permet pas de diff�rencier des histogrammes de valeurs diff�rentes, car s'adapte � la hauteur max des bins
	void Draw(QGraphicsScene&, const GenericPalette & = Palette(QVector<QColor>({Qt::white})), double = 100, double = 100) const;
	
	friend std::ostream& operator<<(std::ostream&, const HistogramD&);
};

inline double HistogramD::operator()(int i) const
{
	return values[i];
}

inline double& HistogramD::operator()(int i)
{
	return values[i];
}

inline int HistogramD::Size() const
{
	return keys.size();
}

inline double HistogramD::GetValue(int i) const
{
	return values[i];
}

inline double HistogramD::GetKey(int i) const
{
	return keys[i];
}

inline void HistogramD::Remove(int i)
{
	keys.remove(i);
	values.remove(i);
}

inline std::ostream& operator<<(std::ostream& os, const HistogramD& h)
{
	os << "Histogram(" << std::endl;
	for (int i = 0; i < h.Size(); ++i)
	{
		os << "\tk=" << h.GetKey(i) << ", v=" << h.GetValue(i) << ",\n";
	}
	os << ")";
	return os;
}
