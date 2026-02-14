#include "qte.h"
#include "vectorset.h"
#include "iso-line-terrain.h"
#include "noise.h"
#include "turbulence.h"
#include "curvepoint.h"
#include "iso-vecto-generation-v1.h"
#include "iso-vecto-generation-v2.h"
#include "iso-vecto-generation-v3.h"
#include "displacement-function.h"
#include "article-utils.h"
#include "eden.h"

#include <algorithm>
#include <random>
#include <QtWidgets/QFileDialog>

using namespace std;

///////////////////////////////
////        USEFUL         ////
///////////////////////////////

RenderOpt render_basic_opt(const IsoLines& isos, const ScalarField2& hf)
{
	double a, b;
	hf.GetRange(a, b);
	QVector<double> v = isos.SortedHeights();
	std::vector<float> heights(v.begin(), v.end());

	if (v[0] < a)
	{
		qDebug() << "[Render Opt] warning: the first iso is lower than the field";
	}

	RenderOpt opt;
	opt.nb_dispatch = 1;
	//opt.nb_dispatch = 4;
	opt.anti_aliasing = 1;
	//opt.anti_aliasing = 2;
	//opt.nb_iso = m_isos.Levels();
	opt.iso_heights = heights;
	opt.sea_level = (v[0] - a) / (b - a);
	opt.terrain_color = Color::Grey(0.9);
	opt.sea_deep_color = Color(209, 234, 255);
	opt.under_shadow_distance = 2000;
	opt.sea_shore_color = opt.sea_deep_color;// *0.2;

	return opt;
}

void save_string_to_file(const QString& text, const QString& filePath)
{
	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qWarning() << "Cannot open file for writing:" << file.errorString();
		return;
	}

	QTextStream out(&file);
	out << text;
	file.close();
}

Polygon2 MainAmplificationWindow::PolygonReunion()
{
	VectorSet2 a({
	  Vector2(0, 0),
	  Vector2(-6, 38),
	  Vector2(7, 52),
	  Vector2(-38, 31),
	  Vector2(-31, 1),
	  Vector2(-18, 24),
	  Vector2(8, 24),
	  Vector2(-4, 33),
	  Vector2(19, 27),
	  Vector2(25, 21),
	  Vector2(22, 28),
	  Vector2(3, 29),
	  Vector2(19, 19),
	  Vector2(-8, 57),
	  Vector2(15, 38),
	  Vector2(37, 29),
	  Vector2(16, 19),
	  Vector2(10, 24),
	  Vector2(41, 11),
	  Vector2(53, 14),
	  Vector2(1, 25),
	  Vector2(95, 52),
	  Vector2(17, -2),
	  Vector2(74, 31),
	  Vector2(59, 9),
	  Vector2(18, 12),
	  Vector2(30, -5),
	  Vector2(22, 7),
	  Vector2(30, -3),
	  Vector2(14, -14),
	  Vector2(34, 1),
	  Vector2(68, -17),
	  Vector2(21, 1),
	  Vector2(26, -19),
	  Vector2(17, -24),
	  Vector2(-1, -62),
	  Vector2(-3, -51),
	  Vector2(15, -60),
	  Vector2(14, -15),
	  Vector2(0, -24),
	  Vector2(13, -4),
	  Vector2(-0, -31),
	  Vector2(-21, -30),
	  Vector2(-23, -21),
	  Vector2(-22, -0),
	  Vector2(-72, -91),
	  Vector2(2, -17),
	  Vector2(-41, -42),
	  Vector2(-1, -37),
	  Vector2(-4, -38),
	  Vector2(-22, -36),
	  Vector2(-43, -38),
	  Vector2(-39, -8),
	  Vector2(-34, -18),
	  Vector2(-41, -7),
	  Vector2(-28, 3),
	  Vector2(-26, -9),
	  Vector2(-23, -8),
	  Vector2(-17, 3),
	  Vector2(-28, 0),
	  Vector2(-24, -16),
	  Vector2(-23, 1),
	  Vector2(-47, 5),
	  Vector2(-16, 13),
	  Vector2(-25, 13),
	  Vector2(-21, 17),
	  Vector2(-16, 21),
	  Vector2(-26, 2),
	  Vector2(-14, 3),
	  Vector2(-18, -3),
	  Vector2(-14, -3),
	});

	a = a.VectorToPoint();
	a.Scale(120.0);

	Box2 b = a.GetBox();
	a.Translate(Vector2(1000.0, 1000.0) - b[0]);
	a.Scale(Vector2(1, -1));
	//a.Reverse();

	return Polygon2(a.Get());
}

Polygon2 MainAmplificationWindow::PolygonHawaii()
{
	VectorSet2 vs({
	  Vector2(0, 0),
	  Vector2(-31, -7),
	  Vector2(-45, 10),
	  Vector2(-29, 16),
	  Vector2(-38, 27),
	  Vector2(-22, -8),
	  Vector2(-16, -15),
	  Vector2(-26, -23),
	  Vector2(-36, -15),
	  Vector2(-9, -36),
	  Vector2(-40, -38),
	  Vector2(-30, -6),
	  Vector2(-23, 6),
	  Vector2(-32, 13),
	  Vector2(-17, 24),
	  Vector2(-85, 24),
	  Vector2(-71, 29),
	  Vector2(-34, 6),
	  Vector2(-50, -9),
	  Vector2(-49, -7),
	  Vector2(-33, -3),
	  Vector2(-44, 24),
	  Vector2(-10, 24),
	  Vector2(-6, 25),
	  Vector2(-42, 64),
	  Vector2(-15, 2),
	  Vector2(-1, 10),
	  Vector2(37, 43),
	  Vector2(15, 2),
	  Vector2(33, 23),
	  Vector2(17, -2),
	  Vector2(32, 29),
	  Vector2(7, 27),
	  Vector2(51, 68),
	  Vector2(27, 41),
	  Vector2(-10, 34),
	  Vector2(7, 35),
	  Vector2(26, 45),
	  Vector2(20, 52),
	  Vector2(52, 57),
	  Vector2(41, 44),
	  Vector2(17, 1),
	  Vector2(12, -2),
	  Vector2(2, -14),
	  Vector2(6, -21),
	  Vector2(26, -36),
	  Vector2(27, -25),
	  Vector2(18, -3),
	  Vector2(15, 4),
	  Vector2(12, -7),
	  Vector2(14, -5),
	  Vector2(0, -23),
	  Vector2(-9, -20),
	  Vector2(3, -10),
	  Vector2(11, 1),
	  Vector2(34, -2),
	  Vector2(32, 4),
	  Vector2(41, -33),
	  Vector2(36, -42),
	  Vector2(33, -59),
	  Vector2(36, -68),
	  Vector2(12, -35),
	  Vector2(9, -37),
	  Vector2(-2, -13),
	  Vector2(2, -11),
	  Vector2(48, -76),
	  Vector2(24, -22),
	  Vector2(12, -33),
	  Vector2(6, -22),
	  Vector2(-5, -18)
		});

	vs = vs.VectorToPoint();
	vs.Scale(100.0);
	Box2 box = vs.GetBox();
	vs.Translate(Vector2(1000.0, 1000.0) - box[0]);
	vs.Reverse();
	vs.Scale(Vector2(1, -1));

	return Polygon2(vs.Get());
}

Polygon2 MainAmplificationWindow::PolygonMan()
{
	VectorSet2 a({
	  Vector2(150.35714,697.71932),
	  Vector2(13.27926,-15.62279),
	  Vector2(16.89931,-19.37721),
	  Vector2(14.59747,-16.47497),
	  Vector2(15.58111,-18.52503),
	  Vector2(4.7381,-7.23127),
	  Vector2(4.72618,-7.76873),
	  Vector2(3.95825,-5.51053),
	  Vector2(5.50604,-9.48947),
	  Vector2(4.56663,-13.58402),
	  Vector2(5.79051,-18.7374),
	  Vector2(4.40165,-15.41597),
	  Vector2(5.95549,-16.90546),
	  Vector2(4.26123,-6.13689),
	  Vector2(5.20306,-6.54168),
	  Vector2(4.30268,-4.96964),
	  Vector2(5.16161,-7.70894),
	  Vector2(2.51942,-9.25068),
	  Vector2(2.83772,-12.53504),
	  Vector2(2.0505,-9.96911),
	  Vector2(3.30664,-11.8166),
	  Vector2(3.89663,-6.63374),
	  Vector2(5.21051,-8.00912),
	  Vector2(3.50735,-6.6113),
	  Vector2(5.5998,-8.03155),
	  Vector2(6.96843,-4.69658),
	  Vector2(9.46014,-5.66057),
	  Vector2(7.43719,-4.55372),
	  Vector2(8.99138,-5.80342),
	  Vector2(5.58944,-4.31912),
	  Vector2(6.91056,-5.50231),
	  Vector2(5.16262,-4.2353),
	  Vector2(7.33738,-5.58612),
	  Vector2(7.49681,-4.59552),
	  Vector2(10.00319,-5.9402),
	  Vector2(7.7019,-5.18518),
	  Vector2(9.7981,-5.35053),
	  Vector2(6.96378,-1.10718),
	  Vector2(8.7505,-0.85711),
	  Vector2(5.67796,-0.27875),
	  Vector2(10.03633,-1.68553),
	  Vector2(14.23692,-5.56933),
	  Vector2(19.51308,-8.18067),
	  Vector2(16.50884,-7.82945),
	  Vector2(17.24116,-5.92055),
	  Vector2(2.3092,0.58325),
	  Vector2(0.54794,0.84532),
	  Vector2(0.71488,-0.66417),
	  Vector2(2.14227,2.09274),
	  Vector2(0.3122,9.84101),
	  Vector2(-0.66935,14.44471),
	  Vector2(0.20614,10.76057),
	  Vector2(-0.56328,13.52514),
	  Vector2(-2.41334,9.61971),
	  Vector2(-3.47952,12.166),
	  Vector2(-2.55573,9.54232),
	  Vector2(-3.33712,12.2434),
	  Vector2(-2.72325,9.45875),
	  Vector2(-3.52675,12.14839),
	  Vector2(-3.20062,9.83191),
	  Vector2(-3.04938,11.77523),
	  Vector2(-0.0905,6.78605),
	  Vector2(0.26911,8.21395),
	  Vector2(-0.34376,6.60466),
	  Vector2(0.52233,8.39534),
	  Vector2(2.76675,6.00571),
	  Vector2(4.01896,7.56572),
	  Vector2(2.60131,6.30628),
	  Vector2(4.18441,7.26515),
	  Vector2(4.96051,3.3424),
	  Vector2(6.64663,3.80045),
	  Vector2(5.02017,3.32559),
	  Vector2(6.58698,3.81727),
	  Vector2(5.37496,1.81935),
	  Vector2(6.94646,2.10922),
	  Vector2(5.61512,1.75866),
	  Vector2(6.70631,2.16992),
	  Vector2(3.93909,1.43094),
	  Vector2(4.81091,1.78334),
	  Vector2(4.18691,1.10384),
	  Vector2(4.56309,2.11045),
	  Vector2(1.37195,2.76281),
	  Vector2(1.30662,3.66576),
	  Vector2(1.69197,2.71825),
	  Vector2(0.98661,3.71032),
	  Vector2(-2.16853,2.45784),
	  Vector2(-3.36719,2.8993),
	  Vector2(-2.33741,2.85527),
	  Vector2(-3.1983,2.50188),
	  Vector2(-2.12929,-0.59058),
	  Vector2(-2.51357,-1.19514),
	  Vector2(-1.84472,-1.6302),
	  Vector2(-2.79814,-0.15551),
	  Vector2(0.2862,4.33073),
	  Vector2(1.49951,6.38355),
	  Vector2(1.49178,4.48388),
	  Vector2(0.29394,6.23041),
	  Vector2(-4.19455,4.78931),
	  Vector2(-6.34117,5.92497),
	  Vector2(-5.39332,4.77188),
	  Vector2(-5.14239,5.94241),
	  Vector2(-0.33347,3.1721),
	  Vector2(0.1549,3.61361),
	  Vector2(0.53052,2.2159),
	  Vector2(-0.70909,4.56982),
	  Vector2(-5.47174,6.62643),
	  Vector2(-8.27826,8.90928),
	  Vector2(-6.99178,6.82199),
	  Vector2(-6.75822,8.71373),
	  Vector2(-0.43826,5.512),
	  Vector2(0.25969,6.63085),
	  Vector2(0.7812,5.15233),
	  Vector2(-0.95977,6.99053),
	  Vector2(-5.99886,4.78991),
	  Vector2(-8.82257,5.7458),
	  Vector2(-7.09897,4.42542),
	  Vector2(-7.72246,6.1103),
	  Vector2(-2.62797,5.1701),
	  Vector2(-2.72917,6.61561),
	  Vector2(-2.93127,5.33224),
	  Vector2(-2.42588,6.45348),
	  Vector2(0.82947,3.46111),
	  Vector2(1.49196,4.03889),
	  Vector2(0.71506,3.35467),
	  Vector2(1.60637,4.14533),
	  Vector2(2.90698,2.50793),
	  Vector2(4.05731,3.02778),
	  Vector2(3.61542,2.28032),
	  Vector2(3.34886,3.2554),
	  Vector2(-0.25079,2.35872),
	  Vector2(-0.82063,2.81985),
	  Vector2(-0.14962,2.15275),
	  Vector2(-0.92181,3.02582),
	  Vector2(-2.77118,2.52296),
	  Vector2(-4.01453,3.19132),
	  Vector2(-3.25958,2.37219),
	  Vector2(-3.52614,3.3421),
	  Vector2(-1.08227,3.03266),
	  Vector2(-1.06059,3.93162),
	  Vector2(-0.44964,3.17493),
	  Vector2(-1.69322,3.78936),
	  Vector2(-3.86,1.37571),
	  Vector2(-5.42571,1.30286),
	  Vector2(-4.56175,0.771),
	  Vector2(-4.72397,1.90758),
	  Vector2(-0.73252,2.64464),
	  Vector2(-0.3389,3.42678),
	  Vector2(-0.033,2.74046),
	  Vector2(-1.03847,3.33097),
	  Vector2(-3.09736,1.34182),
	  Vector2(-4.40264,1.33675),
	  Vector2(-3.36439,1.5103),
	  Vector2(-4.13561,1.16828),
	  Vector2(-2.43611,-0.73841),
	  Vector2(-2.92103,-1.22588),
	  Vector2(-2.42902,-1.29187),
	  Vector2(-2.92813,-0.67241),
	  Vector2(-1.16273,1.3658),
	  Vector2(-1.1587,2.02705),
	  Vector2(-0.61749,1.58356),
	  Vector2(-1.70394,1.8093),
	  Vector2(-3.35755,-0.1054),
	  Vector2(-4.67816,-0.60889),
	  Vector2(-3.28842,-0.98958),
	  Vector2(-4.7473,0.2753),
	  Vector2(-3.4953,4.57961),
	  Vector2(-4.18327,6.84896),
	  Vector2(-4.10343,5.00165),
	  Vector2(-3.57514,6.42692),
	  Vector2(0.89658,4.06167),
	  Vector2(1.78199,4.8669),
	  Vector2(0.98083,4.24327),
	  Vector2(1.69774,4.6853),
	  Vector2(1.93704,1.72881),
	  Vector2(2.52725,1.84262),
	  Vector2(2.5175,1.21409),
	  Vector2(1.94679,2.35734),
	  Vector2(-2.07906,2.00456),
	  Vector2(-3.45665,2.28115),
	  Vector2(-2.43347,2.06635),
	  Vector2(-3.10225,2.21937),
	  Vector2(-2.21519,0.7079),
	  Vector2(-2.78481,0.72067),
	  Vector2(-2.14441,0.42635),
	  Vector2(-2.85559,1.00222),
	  Vector2(-2.27601,1.99665),
	  Vector2(-2.90256,2.82478),
	  Vector2(-2.11297,2.09732),
	  Vector2(-3.06561,2.72411),
	  Vector2(-3.27946,2.1154),
	  Vector2(-4.39911,2.70603),
	  Vector2(-3.17209,2.42642),
	  Vector2(-4.50648,2.39501),
	  Vector2(-4.25659,-0.12504),
	  Vector2(-5.56484,-0.58925),
	  Vector2(-4.61869,-0.75054),
	  Vector2(-5.20274,0.0363),
	  Vector2(-1.92615,1.71456),
	  Vector2(-2.00242,2.39258),
	  Vector2(-1.8774,1.59056),
	  Vector2(-2.05117,2.51658),
	  Vector2(-0.43407,2.97146),
	  Vector2(-0.28021,3.99282),
	  Vector2(0.0926,3.19326),
	  Vector2(-0.80686,3.77103),
	  Vector2(-2.59093,1.56744),
	  Vector2(-3.65907,1.64684),
	  Vector2(-2.97441,1.02039),
	  Vector2(-3.27559,2.1939),
	  Vector2(-0.60543,3.47419),
	  Vector2(-0.28742,4.7401),
	  Vector2(-0.009,3.84145),
	  Vector2(-0.88409,4.37284),
	  Vector2(-2.32262,1.66186),
	  Vector2(-3.21309,1.73099),
	  Vector2(-1.77954,1.565),
	  Vector2(-3.75618,1.82786),
	  Vector2(-6.49304,-0.0961),
	  Vector2(-9.04267,-0.61817),
	  Vector2(-6.14583,-1.38063),
	  Vector2(-9.38989,0.66635),
	  Vector2(-9.06436,7.98942),
	  Vector2(-11.64992,12.01058),
	  Vector2(-10.37325,9.5351),
	  Vector2(-10.34104,10.4649),
	  Vector2(-1.57677,2.66549),
	  Vector2(-0.92323,2.15594),
	  Vector2(-1.50998,1.9933),
	  Vector2(-0.99002,2.8281),
	  Vector2(1.65901,2.0335),
	  Vector2(2.62671,2.4308),
	  Vector2(1.52462,2.4391),
	  Vector2(2.76109,2.0252),
	  Vector2(2.85376,-1.772),
	  Vector2(3.57481,-3.0494),
	  Vector2(3.00714,-3.0165),
	  Vector2(3.42143,-1.805),
	  Vector2(0.30446,1.2237),
	  Vector2(-0.30446,1.6335),
	  Vector2(0.41807,0.094),
	  Vector2(-0.41807,2.7634),
	  Vector2(-4.49475,7.9219),
	  Vector2(-6.93382,11.1852),
	  Vector2(-4.82139,10.3266),
	  Vector2(-6.60718,8.7806),
	  Vector2(-2.20984,-0.593),
	  Vector2(-1.54016,-1.5499),
	  Vector2(-1.77436,-0.1329),
	  Vector2(-1.97564,-2.0099),
	  Vector2(2.04231,-5.1568),
	  Vector2(3.85055,-7.1647),
	  Vector2(3.53125,-5.4047),
	  Vector2(2.3616,-6.9167),
	  Vector2(-2.8538,-3.8434),
	  Vector2(-4.46763,-4.3709),
	  Vector2(-2.94376,-4.1893),
	  Vector2(-4.37767,-4.025),
	  Vector2(-3.93549,0.2532),
	  Vector2(-4.99308,0.9968),
	  Vector2(-3.88334,0.2001),
	  Vector2(-5.04523,1.0499),
	  Vector2(-3.71681,2.8631),
	  Vector2(-4.67605,4.1012),
	  Vector2(-4.31957,3.0199),
	  Vector2(-4.07328,3.9444),
	  Vector2(-0.13374,2.2568),
	  Vector2(0.31231,2.5646),
	  Vector2(0.69545,2.1071),
	  Vector2(-0.51688,2.7144),
	  Vector2(-3.57875,0.2355),
	  Vector2(-5.17125,-0.4141),
	  Vector2(-3.38402,0.9513),
	  Vector2(-5.36598,-1.1299),
	  Vector2(-3.78232,-7.825),
	  Vector2(-4.2534,-11.6393),
	  Vector2(-2.61447,-9.5819),
	  Vector2(-5.42124,-9.88237),
	  Vector2(-7.11352,-1.08517),
	  Vector2(-9.31505,0.0137),
	  Vector2(-7.846561,-0.96459),
	  Vector2(-8.58201,-0.10684),
	  Vector2(-2.77315,1.52024),
	  Vector2(-2.76257,2.05119),
	  Vector2(-2.81003,1.45137),
	  Vector2(-2.72568,2.12005),
	  Vector2(-0.1519,1.61806),
	  Vector2(0.1519,1.95337),
	  Vector2(0.37212,1.62329),
	  Vector2(-0.37212,1.9481),
	  Vector2(-2.06106,0.2598),
	  Vector2(-2.93894,-0.081),
	  Vector2(-2.40247,-0.6061),
	  Vector2(-2.59753,0.7847),
	  Vector2(1.20123,3.0319),
	  Vector2(2.54877,4.111),
	  Vector2(2.23977,3.2323),
	  Vector2(1.51023,3.9105),
	  Vector2(-1.33752,1.5982),
	  Vector2(-2.05534,1.6161),
	  Vector2(-1.22186,1.8311),
	  Vector2(-2.171,1.3832),
	  Vector2(-1.95045,-1.7526),
	  Vector2(-2.33526,-2.8903),
	  Vector2(-1.69572,-2.7296),
	  Vector2(-2.58999,-1.9132),
	  Vector2(-1.16107,1.867),
	  Vector2(-0.98179,2.9544),
	  Vector2(-0.94848,2.0981),
	  Vector2(-1.19438,2.7233),
	  Vector2(-0.86505,2.182),
	  Vector2(-1.09924,2.818),
	  Vector2(-0.30612,2.2785),
	  Vector2(-1.65816,2.7215),
	  Vector2(-4.14939,0.2001),
	  Vector2(-5.85061,-0.3786),
	  Vector2(-5.017629,-0.7729),
	  Vector2(-4.982371,0.5943),
	  Vector2(-0.102792,2.0103),
	  Vector2(0.638506,2.454),
	  Vector2(0.769889,1.9692),
	  Vector2(-0.234174,2.4951),
	  Vector2(-2.845882,0.3379),
	  Vector2(-4.118404,-0.1594),
	  Vector2(-3.307664,0.568),
	  Vector2(-3.656622,-0.3894),
	  Vector2(-0.773623,-2.1757),
	  Vector2(-0.476377,-3.0029),
	  Vector2(-0.224875,-2.3873),
	  Vector2(-1.025125,-2.7912),
	  Vector2(-2.381982,-0.9994),
	  Vector2(-3.3323045,-0.9649),
	  Vector2(-2.836581,-0.1394),
	  Vector2(-2.877704,-1.8249),
	  Vector2(1.686148,-3.9649),
	  Vector2(3.313852,-5.3208),
	  Vector2(2.0173605,-4.5359),
	  Vector2(2.98264,-4.7498),
	  Vector2(2.607391,-1.1473),
	  Vector2(3.285466,-0.9956),
	  Vector2(2.634732,-0.5368),
	  Vector2(3.258125,-1.606),
	  Vector2(1.545357,-3.4816),
	  Vector2(1.668929,-4.91131),
	  Vector2(1.057658,-4.10783),
	  Vector2(2.156628,-4.28503),
	  Vector2(2.785903,-0.70836),
	  Vector2(3.642665,-0.36307),
	  Vector2(3.20733,0.0471),
	  Vector2(3.22124,-1.11847),
	  Vector2(-0.39831,-2.40075),
	  Vector2(-1.20883,-3.13496),
	  Vector2(-0.14136,-2.48613),
	  Vector2(-1.46578,-3.04959),
	  Vector2(-4.131326,-0.64868),
	  Vector2(-5.868675,-0.24418),
	  Vector2(-5.078095,0.34109),
	  Vector2(-4.921905,-1.23395),
	  Vector2(0.463155,-2.56452),
	  Vector2(1.50113,-3.14976),
	  Vector2(0.466057,-2.08611),
	  Vector2(1.498229,-3.62818),
	  Vector2(3.868534,-4.78284),
	  Vector2(5.595751,-6.46716),
	  Vector2(4.110835,-5.80395),
	  Vector2(5.35345,-5.44605),
	  Vector2(3.01713,-0.12816),
	  Vector2(3.41144,0.4853),
	  Vector2(2.87296,0.87825),
	  Vector2(3.55561,-0.5211),
	  Vector2(0.50549,-3.95475),
	  Vector2(-0.14834,-5.68811),
	  Vector2(-0.26442,-4.27987),
	  Vector2(0.62156,-5.36299),
	  Vector2(2.7773,-3.38462),
	  Vector2(4.00842,-4.11538),
	  Vector2(3.57029,-3.29954),
	  Vector2(3.21542,-4.20046),
	  Vector2(-0.27845,-2.3346),
	  Vector2(-0.79298,-2.6654),
	  Vector2(-0.72892,-1.94265),
	  Vector2(-0.34251,-3.05735),
	  Vector2(1.67027,-3.38135),
	  Vector2(2.61545,-4.47579),
	  Vector2(2.35336,-3.41631),
	  Vector2(1.93235,-4.44083),
	  Vector2(-0.97579,-2.97934),
	  Vector2(-1.70278,-3.62781),
	  Vector2(-1.39994,-2.66339),
	  Vector2(-1.27863,-3.94375),
	  Vector2(0.61627,-4.14263),
	  Vector2(1.16945,-5.50023),
	  Vector2(1.33015,-4.34628),
	  Vector2(0.45556,-5.29657),
	  Vector2(-2.3057,-2.74873),
	  Vector2(-3.40859,-3.14413),
	  Vector2(-2.79295,-2.19684),
	  Vector2(-2.92134,-3.69602),
	  Vector2(-0.01,-4.64377),
	  Vector2(0.54549,-6.24908),
	  Vector2(-0.34556,-4.85456),
	  Vector2(0.88127,-6.0383),
	  Vector2(3.80559,-3.44884),
	  Vector2(5.48012,-4.05116),
	  Vector2(4.08104,-2.50457),
	  Vector2(5.20468,-4.99543),
	  Vector2(2.78091,-8.42406),
	  Vector2(3.11195,-11.75451),
	  Vector2(3.42192,-9.49454),
	  Vector2(2.47094,-10.68404),
	  Vector2(-1.32592,-3.94135),
	  Vector2(-2.06694,-4.09436),
	  Vector2(-2.16873,-3.66703),
	  Vector2(-1.22413,-4.36868),
	  Vector2(1.87119,-1.56492),
	  Vector2(2.77167,-1.47079),
	  Vector2(2.12596,-1.03296),
	  Vector2(2.5169,-2.00276),
	  Vector2(0.979431,-3.14982),
	  Vector2(0.98486,-4.35018),
	  Vector2(0.60355,-3.39241),
	  Vector2(1.36073,-4.10759),
	  Vector2(2.39354,-2.35514),
	  Vector2(3.32074,-2.82343),
	  Vector2(2.39113,-2.88552),
	  Vector2(3.32316,-2.29306),
	  Vector2(2.01872,1.25861),
	  Vector2(2.26699,2.13425),
	  Vector2(1.51917,1.99851),
	  Vector2(2.76655,1.39435),
	  Vector2(2.58863,-2.58965),
	  Vector2(3.12565,-4.19606),
	  Vector2(2.41168,-3.46026),
	  Vector2(3.30261,-3.32546),
	  Vector2(2.34327,-0.11139),
	  Vector2(2.8353,0.28996),
	  Vector2(2.36414,0.37101),
	  Vector2(2.81443,-0.19244),
	  Vector2(1.27887,-1.54153),
	  Vector2(1.3997,-2.20847),
	  Vector2(0.52555,-0.86048)
		});

	a = a.VectorToPoint();
	a.Scale(120.0);
	a.Scale(Vector2(1, -1));

	return Polygon2(a.Get());
}

IsoLines input_teaser_article()
{
	Polygon2 p1({
	   Vector2(-274 ,  -320),
	   Vector2(-276 ,  -320),
	   Vector2(-280 ,  -320),
	   Vector2(-284 ,  -320),
	   Vector2(-288 ,  -320),
	   Vector2(-290 ,  -320),
	   Vector2(-292 ,  -320),
	   Vector2(-294 ,  -318),
	   Vector2(-298 ,  -318),
	   Vector2(-300 ,  -316),
	   Vector2(-302 ,  -316),
	   Vector2(-306 ,  -314),
	   Vector2(-310 ,  -314),
	   Vector2(-316 ,  -310),
	   Vector2(-320 ,  -308),
	   Vector2(-324 ,  -306),
	   Vector2(-328 ,  -304),
	   Vector2(-334 ,  -300),
	   Vector2(-338 ,  -298),
	   Vector2(-342 ,  -294),
	   Vector2(-346 ,  -292),
	   Vector2(-352 ,  -290),
	   Vector2(-354 ,  -288),
	   Vector2(-358 ,  -286),
	   Vector2(-358 ,  -284),
	   Vector2(-360 ,  -282),
	   Vector2(-362 ,  -280),
	   Vector2(-362 ,  -278),
	   Vector2(-364 ,  -276),
	   Vector2(-366 ,  -274),
	   Vector2(-366 ,  -270),
	   Vector2(-366 ,  -266),
	   Vector2(-368 ,  -262),
	   Vector2(-368 ,  -258),
	   Vector2(-370 ,  -254),
	   Vector2(-370 ,  -250),
	   Vector2(-372 ,  -244),
	   Vector2(-372 ,  -240),
	   Vector2(-374 ,  -234),
	   Vector2(-374 ,  -228),
	   Vector2(-374 ,  -222),
	   Vector2(-374 ,  -220),
	   Vector2(-374 ,  -216),
	   Vector2(-374 ,  -210),
	   Vector2(-372 ,  -204),
	   Vector2(-370 ,  -198),
	   Vector2(-366 ,  -192),
	   Vector2(-364 ,  -186),
	   Vector2(-362 ,  -182),
	   Vector2(-358 ,  -178),
	   Vector2(-358 ,  -176),
	   Vector2(-354 ,  -172),
	   Vector2(-352 ,  -166),
	   Vector2(-346 ,  -162),
	   Vector2(-344 ,  -158),
	   Vector2(-340 ,  -154),
	   Vector2(-336 ,  -150),
	   Vector2(-332 ,  -146),
	   Vector2(-328 ,  -140),
	   Vector2(-324 ,  -138),
	   Vector2(-320 ,  -132),
	   Vector2(-314 ,  -128),
	   Vector2(-310 ,  -124),
	   Vector2(-304 ,  -118),
	   Vector2(-298 ,  -114),
	   Vector2(-292 ,  -110),
	   Vector2(-288 ,  -106),
	   Vector2(-284 ,  -102),
	   Vector2(-284 ,  -100),
	   Vector2(-278 ,  -96),
	   Vector2(-274 ,  -90),
	   Vector2(-268 ,  -84),
	   Vector2(-266 ,  -78),
	   Vector2(-260 ,  -72),
	   Vector2(-258 ,  -66),
	   Vector2(-256 ,  -66),
	   Vector2(-252 ,  -60),
	   Vector2(-250 ,  -54),
	   Vector2(-246 ,  -48),
	   Vector2(-242 ,  -40),
	   Vector2(-240 ,  -34),
	   Vector2(-238 ,  -28),
	   Vector2(-236 ,  -22),
	   Vector2(-236 ,  -16),
	   Vector2(-234 ,  -10),
	   Vector2(-232 ,  -4),
	   Vector2(-230 ,  4),
	   Vector2(-230 ,  12),
	   Vector2(-230 ,  16),
	   Vector2(-230 ,  24),
	   Vector2(-230 ,  32),
	   Vector2(-230 ,  40),
	   Vector2(-230 ,  46),
	   Vector2(-232 ,  54),
	   Vector2(-234 ,  60),
	   Vector2(-236 ,  66),
	   Vector2(-238 ,  72),
	   Vector2(-238 ,  76),
	   Vector2(-240 ,  82),
	   Vector2(-242 ,  86),
	   Vector2(-244 ,  92),
	   Vector2(-246 ,  98),
	   Vector2(-250 ,  104),
	   Vector2(-252 ,  110),
	   Vector2(-254 ,  116),
	   Vector2(-256 ,  122),
	   Vector2(-256 ,  130),
	   Vector2(-258 ,  134),
	   Vector2(-258 ,  140),
	   Vector2(-260 ,  146),
	   Vector2(-260 ,  150),
	   Vector2(-262 ,  158),
	   Vector2(-262 ,  164),
	   Vector2(-262 ,  170),
	   Vector2(-262 ,  178),
	   Vector2(-264 ,  184),
	   Vector2(-264 ,  190),
	   Vector2(-264 ,  196),
	   Vector2(-264 ,  202),
	   Vector2(-264 ,  204),
	   Vector2(-264 ,  210),
	   Vector2(-262 ,  218),
	   Vector2(-258 ,  224),
	   Vector2(-256 ,  232),
	   Vector2(-252 ,  238),
	   Vector2(-250 ,  246),
	   Vector2(-248 ,  254),
	   Vector2(-242 ,  262),
	   Vector2(-238 ,  270),
	   Vector2(-232 ,  280),
	   Vector2(-228 ,  288),
	   Vector2(-222 ,  294),
	   Vector2(-216 ,  304),
	   Vector2(-210 ,  310),
	   Vector2(-204 ,  318),
	   Vector2(-196 ,  324),
	   Vector2(-186 ,  332),
	   Vector2(-176 ,  338),
	   Vector2(-164 ,  346),
	   Vector2(-152 ,  352),
	   Vector2(-140 ,  358),
	   Vector2(-128 ,  362),
	   Vector2(-114 ,  366),
	   Vector2(-112 ,  366),
	   Vector2(-98 ,  370),
	   Vector2(-86 ,  372),
	   Vector2(-84 ,  372),
	   Vector2(-70 ,  372),
	   Vector2(-60 ,  372),
	   Vector2(-52 ,  372),
	   Vector2(-42 ,  372),
	   Vector2(-40 ,  372),
	   Vector2(-30 ,  370),
	   Vector2(-22 ,  368),
	   Vector2(-16 ,  364),
	   Vector2(-8 ,  360),
	   Vector2(-2 ,  356),
	   Vector2(2 ,  350),
	   Vector2(6 ,  344),
	   Vector2(8 ,  338),
	   Vector2(10 ,  330),
	   Vector2(14 ,  322),
	   Vector2(16 ,  314),
	   Vector2(18 ,  306),
	   Vector2(22 ,  298),
	   Vector2(26 ,  292),
	   Vector2(32 ,  288),
	   Vector2(32 ,  286),
	   Vector2(38 ,  280),
	   Vector2(42 ,  274),
	   Vector2(50 ,  270),
	   Vector2(56 ,  266),
	   Vector2(64 ,  262),
	   Vector2(72 ,  258),
	   Vector2(80 ,  258),
	   Vector2(88 ,  256),
	   Vector2(96 ,  256),
	   Vector2(106 ,  256),
	   Vector2(114 ,  256),
	   Vector2(124 ,  256),
	   Vector2(132 ,  256),
	   Vector2(138 ,  258),
	   Vector2(146 ,  262),
	   Vector2(152 ,  266),
	   Vector2(158 ,  272),
	   Vector2(166 ,  276),
	   Vector2(172 ,  282),
	   Vector2(174 ,  282),
	   Vector2(180 ,  288),
	   Vector2(186 ,  290),
	   Vector2(186 ,  292),
	   Vector2(194 ,  296),
	   Vector2(198 ,  298),
	   Vector2(206 ,  302),
	   Vector2(208 ,  302),
	   Vector2(214 ,  304),
	   Vector2(216 ,  304),
	   Vector2(222 ,  304),
	   Vector2(226 ,  304),
	   Vector2(230 ,  304),
	   Vector2(236 ,  300),
	   Vector2(242 ,  298),
	   Vector2(248 ,  296),
	   Vector2(254 ,  290),
	   Vector2(256 ,  290),
	   Vector2(264 ,  284),
	   Vector2(270 ,  278),
	   Vector2(282 ,  270),
	   Vector2(288 ,  264),
	   Vector2(302 ,  254),
	   Vector2(310 ,  244),
	   Vector2(326 ,  232),
	   Vector2(336 ,  224),
	   Vector2(346 ,  214),
	   Vector2(354 ,  206),
	   Vector2(360 ,  198),
	   Vector2(366 ,  192),
	   Vector2(372 ,  184),
	   Vector2(376 ,  176),
	   Vector2(380 ,  168),
	   Vector2(382 ,  160),
	   Vector2(384 ,  152),
	   Vector2(384 ,  150),
	   Vector2(386 ,  144),
	   Vector2(388 ,  136),
	   Vector2(388 ,  128),
	   Vector2(390 ,  120),
	   Vector2(392 ,  112),
	   Vector2(392 ,  102),
	   Vector2(392 ,  96),
	   Vector2(392 ,  90),
	   Vector2(392 ,  86),
	   Vector2(392 ,  82),
	   Vector2(392 ,  74),
	   Vector2(392 ,  66),
	   Vector2(392 ,  56),
	   Vector2(392 ,  48),
	   Vector2(392 ,  38),
	   Vector2(392 ,  30),
	   Vector2(390 ,  20),
	   Vector2(388 ,  10),
	   Vector2(386 ,  0),
	   Vector2(382 ,  -8),
	   Vector2(376 ,  -20),
	   Vector2(372 ,  -30),
	   Vector2(364 ,  -42),
	   Vector2(358 ,  -52),
	   Vector2(350 ,  -60),
	   Vector2(342 ,  -72),
	   Vector2(336 ,  -80),
	   Vector2(328 ,  -88),
	   Vector2(322 ,  -94),
	   Vector2(314 ,  -98),
	   Vector2(310 ,  -106),
	   Vector2(304 ,  -112),
	   Vector2(298 ,  -118),
	   Vector2(294 ,  -124),
	   Vector2(290 ,  -130),
	   Vector2(286 ,  -136),
	   Vector2(282 ,  -142),
	   Vector2(282 ,  -144),
	   Vector2(280 ,  -152),
	   Vector2(276 ,  -158),
	   Vector2(274 ,  -164),
	   Vector2(274 ,  -168),
	   Vector2(272 ,  -174),
	   Vector2(270 ,  -180),
	   Vector2(268 ,  -190),
	   Vector2(268 ,  -198),
	   Vector2(268 ,  -206),
	   Vector2(268 ,  -214),
	   Vector2(268 ,  -224),
	   Vector2(268 ,  -234),
	   Vector2(268 ,  -242),
	   Vector2(270 ,  -254),
	   Vector2(272 ,  -262),
	   Vector2(276 ,  -270),
	   Vector2(278 ,  -278),
	   Vector2(280 ,  -288),
	   Vector2(282 ,  -296),
	   Vector2(286 ,  -306),
	   Vector2(286 ,  -316),
	   Vector2(288 ,  -322),
	   Vector2(290 ,  -330),
	   Vector2(290 ,  -340),
	   Vector2(290 ,  -342),
	   Vector2(290 ,  -348),
	   Vector2(290 ,  -356),
	   Vector2(288 ,  -362),
	   Vector2(284 ,  -370),
	   Vector2(282 ,  -376),
	   Vector2(278 ,  -382),
	   Vector2(272 ,  -388),
	   Vector2(266 ,  -394),
	   Vector2(260 ,  -400),
	   Vector2(250 ,  -404),
	   Vector2(246 ,  -408),
	   Vector2(240 ,  -412),
	   Vector2(238 ,  -412),
	   Vector2(234 ,  -414),
	   Vector2(232 ,  -416),
	   Vector2(230 ,  -418),
	   Vector2(228 ,  -418),
	   Vector2(224 ,  -418),
	   Vector2(222 ,  -420),
	   Vector2(218 ,  -422),
	   Vector2(214 ,  -422),
	   Vector2(210 ,  -422),
	   Vector2(208 ,  -422),
	   Vector2(204 ,  -422),
	   Vector2(200 ,  -420),
	   Vector2(198 ,  -418),
	   Vector2(192 ,  -414),
	   Vector2(186 ,  -410),
	   Vector2(182 ,  -404),
	   Vector2(176 ,  -400),
	   Vector2(168 ,  -394),
	   Vector2(164 ,  -388),
	   Vector2(160 ,  -384),
	   Vector2(156 ,  -380),
	   Vector2(152 ,  -376),
	   Vector2(146 ,  -372),
	   Vector2(140 ,  -368),
	   Vector2(134 ,  -366),
	   Vector2(128 ,  -364),
	   Vector2(128 ,  -362),
	   Vector2(120 ,  -360),
	   Vector2(114 ,  -356),
	   Vector2(106 ,  -354),
	   Vector2(98 ,  -352),
	   Vector2(92 ,  -352),
	   Vector2(84 ,  -350),
	   Vector2(76 ,  -350),
	   Vector2(68 ,  -350),
	   Vector2(58 ,  -348),
	   Vector2(50 ,  -348),
	   Vector2(42 ,  -348),
	   Vector2(32 ,  -348),
	   Vector2(26 ,  -348),
	   Vector2(18 ,  -348),
	   Vector2(12 ,  -348),
	   Vector2(4 ,  -350),
	   Vector2(-2 ,  -352),
	   Vector2(-10 ,  -354),
	   Vector2(-12 ,  -354),
	   Vector2(-20 ,  -356),
	   Vector2(-26 ,  -360),
	   Vector2(-34 ,  -362),
	   Vector2(-42 ,  -364),
	   Vector2(-42 ,  -366),
	   Vector2(-48 ,  -368),
	   Vector2(-54 ,  -368),
	   Vector2(-60 ,  -370),
	   Vector2(-64 ,  -372),
	   Vector2(-68 ,  -372),
	   Vector2(-72 ,  -374),
	   Vector2(-76 ,  -374),
	   Vector2(-80 ,  -374),
	   Vector2(-82 ,  -376),
	   Vector2(-84 ,  -376),
	   Vector2(-88 ,  -376),
	   Vector2(-92 ,  -376),
	   Vector2(-96 ,  -376),
	   Vector2(-102 ,  -376),
	   Vector2(-106 ,  -376),
	   Vector2(-112 ,  -376),
	   Vector2(-116 ,  -376),
	   Vector2(-122 ,  -376),
	   Vector2(-126 ,  -376),
	   Vector2(-130 ,  -374),
	   Vector2(-134 ,  -372),
	   Vector2(-138 ,  -370),
	   Vector2(-142 ,  -368),
	   Vector2(-146 ,  -366),
	   Vector2(-150 ,  -364),
	   Vector2(-156 ,  -362),
	   Vector2(-160 ,  -360),
	   Vector2(-166 ,  -356),
	   Vector2(-170 ,  -356),
	   Vector2(-174 ,  -354),
	   Vector2(-176 ,  -352),
	   Vector2(-182 ,  -348),
	   Vector2(-184 ,  -348),
	   Vector2(-188 ,  -346),
	   Vector2(-190 ,  -346),
	   Vector2(-192 ,  -346),
	   Vector2(-196 ,  -344),
	   Vector2(-198 ,  -344),
	   Vector2(-198 ,  -342),
	   Vector2(-200 ,  -342),
	});
	Polygon2 p2({
		Vector2(-104 ,  -232),
		Vector2(-104 ,  -230),
		Vector2(-104 ,  -228),
		Vector2(-106 ,  -228),
		Vector2(-106 ,  -226),
		Vector2(-106 ,  -224),
		Vector2(-106 ,  -222),
		Vector2(-108 ,  -220),
		Vector2(-108 ,  -218),
		Vector2(-110 ,  -214),
		Vector2(-112 ,  -212),
		Vector2(-112 ,  -210),
		Vector2(-112 ,  -208),
		Vector2(-114 ,  -206),
		Vector2(-114 ,  -204),
		Vector2(-114 ,  -200),
		Vector2(-114 ,  -198),
		Vector2(-114 ,  -196),
		Vector2(-114 ,  -194),
		Vector2(-114 ,  -190),
		Vector2(-114 ,  -188),
		Vector2(-114 ,  -182),
		Vector2(-112 ,  -180),
		Vector2(-112 ,  -174),
		Vector2(-110 ,  -170),
		Vector2(-108 ,  -166),
		Vector2(-106 ,  -164),
		Vector2(-102 ,  -158),
		Vector2(-100 ,  -154),
		Vector2(-96 ,  -150),
		Vector2(-92 ,  -148),
		Vector2(-90 ,  -146),
		Vector2(-88 ,  -144),
		Vector2(-86 ,  -140),
		Vector2(-84 ,  -140),
		Vector2(-84 ,  -138),
		Vector2(-80 ,  -136),
		Vector2(-80 ,  -134),
		Vector2(-78 ,  -132),
		Vector2(-78 ,  -130),
		Vector2(-76 ,  -128),
		Vector2(-74 ,  -124),
		Vector2(-72 ,  -120),
		Vector2(-70 ,  -120),
		Vector2(-70 ,  -114),
		Vector2(-66 ,  -110),
		Vector2(-62 ,  -106),
		Vector2(-56 ,  -100),
		Vector2(-56 ,  -94),
		Vector2(-52 ,  -90),
		Vector2(-50 ,  -84),
		Vector2(-50 ,  -80),
		Vector2(-48 ,  -72),
		Vector2(-46 ,  -68),
		Vector2(-44 ,  -60),
		Vector2(-44 ,  -58),
		Vector2(-44 ,  -50),
		Vector2(-44 ,  -42),
		Vector2(-44 ,  -40),
		Vector2(-44 ,  -34),
		Vector2(-44 ,  -26),
		Vector2(-44 ,  -20),
		Vector2(-46 ,  -20),
		Vector2(-46 ,  -14),
		Vector2(-48 ,  -8),
		Vector2(-48 ,  -2),
		Vector2(-52 ,  4),
		Vector2(-54 ,  10),
		Vector2(-56 ,  16),
		Vector2(-58 ,  16),
		Vector2(-62 ,  22),
		Vector2(-64 ,  28),
		Vector2(-70 ,  34),
		Vector2(-72 ,  40),
		Vector2(-76 ,  46),
		Vector2(-80 ,  50),
		Vector2(-82 ,  54),
		Vector2(-86 ,  58),
		Vector2(-88 ,  62),
		Vector2(-90 ,  66),
		Vector2(-92 ,  70),
		Vector2(-94 ,  72),
		Vector2(-96 ,  78),
		Vector2(-98 ,  80),
		Vector2(-98 ,  84),
		Vector2(-100 ,  88),
		Vector2(-102 ,  92),
		Vector2(-102 ,  94),
		Vector2(-102 ,  96),
		Vector2(-102 ,  98),
		Vector2(-102 ,  102),
		Vector2(-102 ,  106),
		Vector2(-102 ,  110),
		Vector2(-102 ,  114),
		Vector2(-102 ,  118),
		Vector2(-102 ,  122),
		Vector2(-100 ,  126),
		Vector2(-98 ,  132),
		Vector2(-94 ,  134),
		Vector2(-92 ,  138),
		Vector2(-92 ,  140),
		Vector2(-88 ,  144),
		Vector2(-86 ,  146),
		Vector2(-84 ,  150),
		Vector2(-82 ,  152),
		Vector2(-78 ,  156),
		Vector2(-74 ,  158),
		Vector2(-70 ,  162),
		Vector2(-66 ,  164),
		Vector2(-62 ,  166),
		Vector2(-56 ,  168),
		Vector2(-52 ,  170),
		Vector2(-48 ,  170),
		Vector2(-42 ,  170),
		Vector2(-36 ,  170),
		Vector2(-32 ,  170),
		Vector2(-24 ,  170),
		Vector2(-16 ,  170),
		Vector2(-10 ,  170),
		Vector2(-2 ,  170),
		Vector2(2 ,  170),
		Vector2(8 ,  168),
		Vector2(12 ,  166),
		Vector2(16 ,  164),
		Vector2(20 ,  162),
		Vector2(22 ,  160),
		Vector2(24 ,  160),
		Vector2(26 ,  156),
		Vector2(28 ,  152),
		Vector2(30 ,  148),
		Vector2(32 ,  142),
		Vector2(36 ,  136),
		Vector2(38 ,  130),
		Vector2(40 ,  122),
		Vector2(40 ,  112),
		Vector2(42 ,  104),
		Vector2(42 ,  98),
		Vector2(44 ,  92),
		Vector2(46 ,  86),
		Vector2(46 ,  82),
		Vector2(46 ,  78),
		Vector2(48 ,  74),
		Vector2(48 ,  70),
		Vector2(48 ,  64),
		Vector2(48 ,  62),
		Vector2(48 ,  56),
		Vector2(48 ,  52),
		Vector2(50 ,  48),
		Vector2(50 ,  46),
		Vector2(52 ,  42),
		Vector2(54 ,  38),
		Vector2(56 ,  34),
		Vector2(58 ,  30),
		Vector2(62 ,  24),
		Vector2(66 ,  20),
		Vector2(72 ,  16),
		Vector2(80 ,  8),
		Vector2(86 ,  4),
		Vector2(92 ,  0),
		Vector2(92 ,  -2),
		Vector2(100 ,  -6),
		Vector2(106 ,  -12),
		Vector2(108 ,  -12),
		Vector2(114 ,  -18),
		Vector2(118 ,  -22),
		Vector2(122 ,  -26),
		Vector2(128 ,  -34),
		Vector2(134 ,  -40),
		Vector2(138 ,  -46),
		Vector2(144 ,  -54),
		Vector2(148 ,  -60),
		Vector2(152 ,  -66),
		Vector2(152 ,  -68),
		Vector2(158 ,  -72),
		Vector2(158 ,  -74),
		Vector2(160 ,  -78),
		Vector2(162 ,  -82),
		Vector2(166 ,  -88),
		Vector2(168 ,  -94),
		Vector2(170 ,  -100),
		Vector2(174 ,  -106),
		Vector2(176 ,  -112),
		Vector2(178 ,  -118),
		Vector2(178 ,  -122),
		Vector2(180 ,  -128),
		Vector2(182 ,  -134),
		Vector2(182 ,  -138),
		Vector2(182 ,  -144),
		Vector2(184 ,  -152),
		Vector2(184 ,  -156),
		Vector2(184 ,  -160),
		Vector2(184 ,  -166),
		Vector2(182 ,  -170),
		Vector2(180 ,  -176),
		Vector2(178 ,  -180),
		Vector2(174 ,  -184),
		Vector2(172 ,  -190),
		Vector2(170 ,  -192),
		Vector2(168 ,  -198),
		Vector2(166 ,  -200),
		Vector2(164 ,  -200),
		Vector2(162 ,  -202),
		Vector2(160 ,  -206),
		Vector2(158 ,  -206),
		Vector2(156 ,  -208),
		Vector2(154 ,  -208),
		Vector2(152 ,  -210),
		Vector2(148 ,  -210),
		Vector2(146 ,  -210),
		Vector2(144 ,  -210),
		Vector2(142 ,  -210),
		Vector2(138 ,  -210),
		Vector2(136 ,  -210),
		Vector2(134 ,  -210),
		Vector2(132 ,  -210),
		Vector2(128 ,  -210),
		Vector2(126 ,  -210),
		Vector2(124 ,  -210),
		Vector2(120 ,  -210),
		Vector2(116 ,  -210),
		Vector2(112 ,  -210),
		Vector2(110 ,  -210),
		Vector2(106 ,  -210),
		Vector2(104 ,  -210),
		Vector2(100 ,  -210),
		Vector2(96 ,  -210),
		Vector2(92 ,  -210),
		Vector2(88 ,  -212),
		Vector2(84 ,  -214),
		Vector2(80 ,  -216),
		Vector2(78 ,  -216),
		Vector2(76 ,  -216),
		Vector2(72 ,  -218),
		Vector2(70 ,  -220),
		Vector2(66 ,  -222),
		Vector2(62 ,  -224),
		Vector2(56 ,  -226),
		Vector2(52 ,  -230),
		Vector2(48 ,  -232),
		Vector2(44 ,  -236),
		Vector2(38 ,  -240),
		Vector2(36 ,  -240),
		Vector2(32 ,  -242),
		Vector2(26 ,  -246),
		Vector2(20 ,  -248),
		Vector2(14 ,  -252),
		Vector2(8 ,  -254),
		Vector2(4 ,  -256),
		Vector2(-2 ,  -258),
		Vector2(-8 ,  -260),
		Vector2(-10 ,  -260),
		Vector2(-16 ,  -260),
		Vector2(-18 ,  -262),
		Vector2(-22 ,  -262),
		Vector2(-26 ,  -262),
		Vector2(-28 ,  -262),
		Vector2(-30 ,  -262),
		Vector2(-34 ,  -262),
		Vector2(-36 ,  -262),
		Vector2(-40 ,  -262),
		Vector2(-44 ,  -260),
		Vector2(-48 ,  -258),
		Vector2(-50 ,  -256),
		Vector2(-54 ,  -254),
		Vector2(-56 ,  -252),
		Vector2(-60 ,  -250),
		Vector2(-64 ,  -248),
		Vector2(-66 ,  -246),
		Vector2(-68 ,  -244),
		Vector2(-70 ,  -244),
		Vector2(-72 ,  -242),
	});
	Polygon2 p3({
		Vector2(260 ,  16),
		Vector2(258 ,  16),
		Vector2(256 ,  16),
		Vector2(252 ,  16),
		Vector2(250 ,  16),
		Vector2(244 ,  18),
		Vector2(238 ,  18),
		Vector2(232 ,  20),
		Vector2(228 ,  22),
		Vector2(226 ,  22),
		Vector2(220 ,  22),
		Vector2(214 ,  24),
		Vector2(208 ,  24),
		Vector2(204 ,  26),
		Vector2(198 ,  28),
		Vector2(194 ,  30),
		Vector2(190 ,  32),
		Vector2(188 ,  34),
		Vector2(186 ,  34),
		Vector2(184 ,  38),
		Vector2(182 ,  40),
		Vector2(180 ,  42),
		Vector2(176 ,  46),
		Vector2(172 ,  50),
		Vector2(166 ,  56),
		Vector2(160 ,  60),
		Vector2(156 ,  66),
		Vector2(150 ,  70),
		Vector2(146 ,  74),
		Vector2(140 ,  80),
		Vector2(136 ,  84),
		Vector2(136 ,  86),
		Vector2(134 ,  88),
		Vector2(130 ,  92),
		Vector2(128 ,  94),
		Vector2(128 ,  96),
		Vector2(128 ,  98),
		Vector2(128 ,  100),
		Vector2(126 ,  102),
		Vector2(126 ,  106),
		Vector2(126 ,  108),
		Vector2(126 ,  112),
		Vector2(126 ,  114),
		Vector2(126 ,  118),
		Vector2(126 ,  122),
		Vector2(126 ,  128),
		Vector2(126 ,  132),
		Vector2(126 ,  136),
		Vector2(128 ,  140),
		Vector2(130 ,  146),
		Vector2(134 ,  152),
		Vector2(134 ,  156),
		Vector2(138 ,  160),
		Vector2(142 ,  164),
		Vector2(144 ,  170),
		Vector2(148 ,  172),
		Vector2(150 ,  176),
		Vector2(150 ,  178),
		Vector2(154 ,  180),
		Vector2(154 ,  182),
		Vector2(156 ,  184),
		Vector2(156 ,  186),
		Vector2(158 ,  186),
		Vector2(158 ,  188),
		Vector2(160 ,  190),
		Vector2(162 ,  192),
		Vector2(164 ,  194),
		Vector2(166 ,  196),
		Vector2(166 ,  198),
		Vector2(170 ,  200),
		Vector2(172 ,  200),
		Vector2(172 ,  202),
		Vector2(174 ,  204),
		Vector2(176 ,  204),
		Vector2(178 ,  204),
		Vector2(178 ,  206),
		Vector2(180 ,  206),
		Vector2(182 ,  208),
		Vector2(184 ,  208),
		Vector2(188 ,  210),
		Vector2(192 ,  210),
		Vector2(196 ,  210),
		Vector2(202 ,  210),
		Vector2(206 ,  210),
		Vector2(210 ,  212),
		Vector2(216 ,  212),
		Vector2(222 ,  212),
		Vector2(228 ,  212),
		Vector2(234 ,  212),
		Vector2(240 ,  210),
		Vector2(246 ,  206),
		Vector2(250 ,  204),
		Vector2(252 ,  202),
		Vector2(254 ,  200),
		Vector2(258 ,  198),
		Vector2(260 ,  196),
		Vector2(260 ,  192),
		Vector2(264 ,  190),
		Vector2(266 ,  186),
		Vector2(268 ,  182),
		Vector2(270 ,  180),
		Vector2(270 ,  174),
		Vector2(272 ,  174),
		Vector2(274 ,  170),
		Vector2(276 ,  166),
		Vector2(278 ,  162),
		Vector2(278 ,  160),
		Vector2(282 ,  156),
		Vector2(282 ,  152),
		Vector2(284 ,  150),
		Vector2(286 ,  148),
		Vector2(286 ,  146),
		Vector2(288 ,  144),
		Vector2(290 ,  142),
		Vector2(290 ,  140),
		Vector2(292 ,  140),
		Vector2(292 ,  138),
		Vector2(294 ,  136),
		Vector2(294 ,  134),
		Vector2(296 ,  132),
		Vector2(298 ,  132),
		Vector2(300 ,  130),
		Vector2(300 ,  128),
		Vector2(304 ,  126),
		Vector2(308 ,  124),
		Vector2(310 ,  122),
		Vector2(316 ,  118),
		Vector2(320 ,  114),
		Vector2(322 ,  112),
		Vector2(324 ,  112),
		Vector2(326 ,  110),
		Vector2(326 ,  108),
		Vector2(328 ,  106),
		Vector2(328 ,  104),
		Vector2(332 ,  100),
		Vector2(332 ,  98),
		Vector2(334 ,  94),
		Vector2(336 ,  90),
		Vector2(336 ,  86),
		Vector2(338 ,  82),
		Vector2(340 ,  76),
		Vector2(342 ,  70),
		Vector2(342 ,  64),
		Vector2(344 ,  58),
		Vector2(344 ,  52),
		Vector2(344 ,  48),
		Vector2(344 ,  42),
		Vector2(344 ,  36),
		Vector2(344 ,  32),
		Vector2(344 ,  28),
		Vector2(344 ,  24),
		Vector2(342 ,  20),
		Vector2(340 ,  18),
		Vector2(340 ,  16),
		Vector2(336 ,  14),
		Vector2(336 ,  12),
		Vector2(334 ,  8),
		Vector2(332 ,  6),
		Vector2(330 ,  4),
		Vector2(328 ,  2),
		Vector2(328 ,  0),
		Vector2(326 ,  -2),
		Vector2(324 ,  -2),
		Vector2(320 ,  -4),
		Vector2(318 ,  -4),
		Vector2(314 ,  -6),
		Vector2(312 ,  -6),
		Vector2(308 ,  -6),
		Vector2(306 ,  -6),
		Vector2(304 ,  -6),
		Vector2(302 ,  -6),
		Vector2(300 ,  -6),
		Vector2(298 ,  -6),
		Vector2(294 ,  -6),
		Vector2(290 ,  -6),
		Vector2(288 ,  -6),
		Vector2(286 ,  -6),
	});

	IsoLinePoly ilp1 = IsoLinePoly(p1, 0).Smooth(10, 0.1, 7);
	IsoLinePoly ilp2 = IsoLinePoly(p2, 1000).Smooth(10, 0.1, 7);
	IsoLinePoly ilp3 = IsoLinePoly(p3, 1000).Smooth(10, 0.1, 7);

	ilp1.Scale(32);
	ilp2.Scale(32);
	ilp3.Scale(32);

	return IsoLines({ilp1, ilp2, ilp3});
}

ScalarField2 polygon_mask(const Polygon2& poly, int w, int h, double scaleFactor = 1.1)
{
	//Box2 b = poly.GetBox();
	ScalarField2 mask(poly.GetBox().ScaledCentered(scaleFactor), w, h, 0);
	for (int x = 0; x < mask.GetSizeX(); ++x)
	{
		for (int y = 0; y < mask.GetSizeY(); ++y)
		{
			Vector2 c = mask.ArrayVertex(x, y);
			if (poly.Inside(c))
				mask(x, y) = 1;
		}
	}
	return mask;
}

IsoLines isolines_mask(const ScalarField2& mask)
{
	QSet<double> values;
	for (double v : mask)
		values.insert(v);

	return IsoLines(mask, values);
}

QGraphicsScene* isolines_mask_scene(const ScalarField2& mask, GenericPalette* isoPalette, bool withBackground, bool withBorder)
{
	IsoLines isos = isolines_mask(mask);
	IsoLines::DisplayOptions opt;
	opt.palette = isoPalette;
	opt.fill = true;
	QGraphicsScene* s = isos.ToScene(opt);
	if (withBackground)
	{
		auto rect = s->addRect(mask.GetBox().GetQtRect(), QPen(Qt::white), QBrush(Qt::white));
		rect->setZValue(-1000);
	}
	if (withBorder)
	{
		double l = Norm(mask.GetBox().Diagonal()) / 250;
		QPen p(Qt::black);
		p.setWidth(l);
		s->addRect(mask.GetBox().GetQtRect(), QPen(p));
	}
	return s;
}

ScalarField2 thresholding(const ScalarField2& sf, int n, double sea = 0.01, int smooth_number = 7)
{
	double step = (1 - sea) / n;
	QSet<double> values;
	for (int i = 0; i < n; ++i)
		values.insert(sea + step * i);

	ScalarField2 result = sf;
	result.Smooth(smooth_number);
	result.Normalize();
	
	for (int i = 0; i < sf.VertexSize(); ++i)
	{
		double vr = result[i];
		double closest = 0;
		for (double v : values)
			if (vr > v && vr - v < vr - closest)
				closest = v;

		if (closest == 0)
			closest -= step + sea;

		result[i] = closest;
	}
	result.Normalize();

	return result;
}

ScalarField2 mask_from_isos(const Box2& b, const IsoLines& isos, int w, int h)
{
	IsoLineTerrain ilt(isos);
	ScalarField2 sf = ilt.StairsField(b, w, h);
	sf.Normalize(); // Pour avoir 0 au niveau de la mer
	return sf;
}

ScalarField2 uniform(int w, int h, Box2 b = Box2(0.5))
{
	ScalarField2 sf(b, w, h, 1);
	return sf;
}

ScalarField2 perlin(int w, int h, double lambda = 1, const Box2& b = Box2(.5))
{
	Noise2 t;
	ScalarField2 sf(b, w, h);
	for (int y = 0; y < w; ++y)
	{
		for (int x = 0; x < h; ++x)
		{
			double v = Math::Abs(t.Value(Vector2(x * (lambda / w), y * (lambda / h))));
			sf(x, y) = v;
		}
	}
	sf.Normalize();

	return sf;
}

ScalarField2 fbm(int w, int h, double lambda = 1, const Box2& b = Box2(.5))
{
	NoiseTurbulence2 t;
	ScalarField2 sf(b, w, h);
	for (int y = 0; y < w; ++y)
	{
		for (int x = 0; x < h; ++x)
		{
			double v = Math::Abs(t.Value(Vector2(x * (lambda / w), y * (lambda / h))));
			sf(x, y) = v;
		}
	}
	sf.Normalize();

	return sf;
}

ScalarField2 user_defined(int x, int y, QString name, Qt::TransformationMode transformMode = Qt::SmoothTransformation)
{
	QImage i("../images/input/" + name);
	ScalarField2 sf(i.scaled(x, y, Qt::IgnoreAspectRatio, transformMode));
	sf.Normalize();
	sf.Symmetry(false, true);
	//sf.Pow(10);
	return sf;
}

ScalarField2 user_defined_fbm(int x, int y, QString name, double lambda = 1)
{
	ScalarField2 sf1 = user_defined(x, y, name);
	ScalarField2 sf2 = fbm(x, y, lambda);
	ScalarField2 sf = sf1 + .5 * sf2;
	sf.Normalize();
	return sf;
}

ScalarField2 change_sf_box(const ScalarField2& sf, const Box2& b)
{
	int x = sf.GetSizeX();
	int y = sf.GetSizeY();
	ScalarField2 nsf = ScalarField2(b, x, y);
	for (int i = 0; i < x; ++i)
		for (int j = 0; j < y; ++j)
			nsf(i, j) = sf.at(i, j);
	return nsf;
}

void get_pixel_sizes_for_box(const Box2& b, int max_x, int max_y, int& x, int& y)
{
	double w = b.Width();
	double h = b.Height();
	double ratio = w / h;
	x = max_x;
	y = max_y;
	if (ratio < 1)
		x *= ratio;
	else
		y /= ratio;
}

HistogramD histo_from_hf(const HeightField& hf, int nbBins)
{
	double epsilon = 0.001;
	double a, b;
	hf.GetRange(a, b);
	return HistogramD(hf, nbBins, a + epsilon, b); // a + epsilon pour remove la mer
}

// Fonction entre [0, 1] -> R
HistogramD histo_from_function(const function<double(double)>& f, int nbBins, double minHeight = 0, double maxHeight = 1)
{
	QVector<double> keys;
	QVector<double> values;
	keys.reserve(nbBins);
	values.reserve(nbBins);

	for (int i = 0; i < nbBins; ++i)
	{
		double u = (2. * i + 1) / (2. * nbBins);
		double v = f(u);
		double h = Math::Lerp(minHeight, maxHeight, i / (nbBins - 1.));
		keys.append(h);
		values.append(v);
	}

	return HistogramD(keys, values);
}

// Fonctions utilisées dans l'article
// 0: f(x) = x
// 1: f(x) = 1 - x
// 2: f(x) = e(4x)
// 3: f(x) = e(4(1 - x))
// 4: f(x) = sin(xπ)
// 5: f(x) = 1.2 + sin((1 + x)π)
// 6: f(x) = 1
// 7: f(x) = 1.2 + sin((4 + x)*.75π)
HistogramD common_histo_from_function(int id, int nbBins, double minHeight = 0, double maxHeight = 1)
{
	if (id == 0)
		return histo_from_function([](double x) { return x; }, nbBins, minHeight, maxHeight);
	if (id == 1)
		return histo_from_function([](double x) { return 1 - x; }, nbBins, minHeight, maxHeight);
	if (id == 2)
		return histo_from_function([](double x) { return exp(4 * x); }, nbBins, minHeight, maxHeight);
	if (id == 3)
		return histo_from_function([](double x) { return exp(4 * (1 - x)); }, nbBins, minHeight, maxHeight);
	if (id == 4)
		return histo_from_function([](double x) { return sin(x * M_PI); }, nbBins, minHeight, maxHeight);
	if (id == 5)
		return histo_from_function([](double x) { return 1.2 + sin((1 + x) * M_PI); }, nbBins, minHeight, maxHeight);
	if (id == 6)
		return histo_from_function([](double x) { return 1; }, nbBins, minHeight, maxHeight);
	if (id == 7)
		return histo_from_function([](double x) { return 1.2 + sin((4 + x) * .75 * M_PI); }, nbBins, minHeight, maxHeight);

	return common_histo_from_function(0, nbBins, minHeight, maxHeight);
}

Polygon2 contour_from_mask(const HeightField& mask)
{
	return mask.LineSegments(0).GetPolygons().At(0);
}

QImage proba_inside_iso(const ScalarField2& proba, const Polygon2& iso, const GenericPalette& palette = AnalyticPalette(0))
{
	ScalarField2 img(proba, 0);
	for (int i = 0; i < proba.GetSizeX(); ++i)
	{
		for (int j = 0; j < proba.GetSizeY(); ++j)
		{
			Vector2 c = proba.CellCenter(i, j);
			if (iso.Inside(c))
				img(i, j) = proba.at(i, j);
		}
	}

	return img.CreateImage(palette);
}

QImage proba_inside_isos(const ScalarField2& proba, const IsoLines& isos, const GenericPalette& palette = AnalyticPalette(0))
{
	ScalarField2 img(proba, 0);
	for (int i = 0; i < proba.GetSizeX(); ++i)
	{
		for (int j = 0; j < proba.GetSizeY(); ++j)
		{
			Vector2 c = proba.CellCenter(i, j);
			if (isos.Inside(c))
				img(i, j) = proba.at(i, j);
		}
	}

	return img.CreateImage(palette);
}

// HeightFields
HeightField reunion_hf(const Box2& box, int x, int y)
{
	return HeightField(box, QImage("../Data/dems/reunion.png").flipped().scaled(x, y, Qt::IgnoreAspectRatio, Qt::SmoothTransformation), 0, 3070);
}

HeightField rockies_hf(const Box2& box, int x, int y)
{
	return HeightField(box, QImage("../Data/dems/rockies_zoom_breach.png").flipped().scaled(x, y, Qt::IgnoreAspectRatio, Qt::SmoothTransformation), 0, 2500);
}

HeightField iceland_hf(const Box2& box, int x, int y)
{
	return HeightField(box, QImage("../Data/dems/iceland.png").flipped().scaled(x, y, Qt::IgnoreAspectRatio, Qt::SmoothTransformation), 0, 2110);
}

///////////////////////////////
////    REPORT FIGURES     ////
///////////////////////////////

#include "cpu.h"
#include "misc.h"
void MainAmplificationWindow::GenerateFigures()
{
	generate_figures = true;
	QString prefix = "Figures/";

	// 01 Teaser
	qDebug() << "Generate figure 01";
	GenerateFigure1(prefix + "01-teaser/");

	// 02 Overview/Pipeline
	qDebug() << "Generate figure 02";
	GenerateFigure2(prefix + "02-overview/");

	// 05 Different eden growth
	qDebug() << "Generate figure 05";
	GenerateFigure5(prefix + "05-eden-growth/");

	// 06/08 growth steps
	qDebug() << "Generate figure 06/08";
	GenerateFigure6And8(prefix + "06-08-growth-steps/");

	// 07 Delaunay issue and solution
	qDebug() << "Generate figures 07";
	GenerateFigure7(prefix + "07-mikowski/");

	// 10 Extraction issue
	qDebug() << "Generate figure 10";
	GenerateFigure10(prefix + "10-extraction-isos/");

	// 11 River noise
	qDebug() << "Generate figure 11";
	GenerateFigure11(prefix + "11-user-rivers/");

	// 12 Multi-zone boundaries
	qDebug() << "Generate figure 12";
	GenerateFigure12(prefix + "12-double-eden-boundaries/");

	// 13/14 Multi-zones
	qDebug() << "Generate figure 13/14";
	GenerateFigure13And14(prefix + "13-14-multi-zones/");

	// 16/19/20 Different editing tool
	qDebug() << "Generate figure 16/19/20";
	GenerateFigure16And19And20(prefix + "16-19-20-editing-tools/");

	// 21 different amplifications
	qDebug() << "Generate figure 21";
	GenerateFigure21(prefix + "21-amplifications/");

	// 22 Different generation
	qDebug() << "Generate figure 22";
	GenerateFigure22(prefix + "22-results/");

	// 23 comparison change of parameters
	qDebug() << "Generate figure 23";
	GenerateFigure23(prefix + "23-parameters/");

	// 24 contours with more or less details
	qDebug() << "Generate figure 24";
	GenerateFigure24(prefix + "24-detailed-contours/");

	// 25 time taken
	qDebug() << "Generate figure 25 and table 01";
	GenerateFigure25AndTable1(prefix + "25-T1-time-taken/");

	// 26 comparison with orometry and gradient authoring paper
	qDebug() << "Generate figure 26";
	GenerateFigure26(prefix + "26-comparison/");

	// 27 editing session
	qDebug() << "Generate figure 27";
	GenerateFigure27(prefix + "27-editing-session/");

	// 28 comparison real alps
	qDebug() << "Generate figure 28";
	GenerateFigure28(prefix + "28-comparison-real/");

	qDebug() << "End of generation";
	generate_figures = false;
	return;

	// PRESENTATION

	// Presentation 01 motivation
	qDebug() << "Generate presentation figure 01";
	GenerateFigurePresentation1(prefix + "presentation/01-motivation/");

	// Presentation 02 generation
	qDebug() << "Generate presentation figure 02";
	GenerateFigurePresentation2(prefix + "presentation/02-generation/");

	// Presentation 03 noise
	qDebug() << "Generate presentation figure 03";
	GenerateFigurePresentation3(prefix + "presentation/03-noise/");

	// Presentation 04 double-eden
	qDebug() << "Generate presentation figure 04";
	GenerateFigurePresentation4(prefix + "presentation/04-double-eden/");

	// Presentation 05 reconstruction
	qDebug() << "Generate presentation figure 05";
	GenerateFigurePresentation5(prefix + "presentation/05-reconstruction/");

	// Presentation 06 results
	qDebug() << "Generate presentation figure 06";
	GenerateFigurePresentation6(prefix + "presentation/06-results/");

	qDebug() << "End of generation";
	generate_figures = false;
}

void MainAmplificationWindow::GenerateFigure1(const QString& prefix)
{
	QDir dir(prefix);
	if (!dir.exists())
		dir.mkpath(".");

	Box2 b(15000);
	double nb_particles = 10000;
	double min_height = 0;
	double max_height = 5000;
	int nb_isos = 20;

	QVector<QString> masks = {
		"../images/input/teaser-1.png",
		//"../images/input/teaser-2-a.png",
		"../images/input/teaser-2-b.png",
		"../images/input/teaser-3.png",
	};
	
	ChangeBoxes(b);
	ChangeHeights(min_height, max_height);
	ChangeEstimatedNbParticles(nb_particles);
	ChangeHistogramFromFile("../Data/dems/rockies_zoom_breach.png", nb_isos);
	ChangeNoise(uniform(256, 256));
	ChangeCamera(190, 40, 60000);

	ExportHistogram(prefix + "histogram.svg");
	for (int i = 0; i < masks.size(); ++i)
	{
		ChangeMaskFromMaskFile(masks[i]);
		
		Generate();

		ExportMaskSVG(prefix + "teaser_" + QString::number(i) + "_input.svg");
		ExportIsosSVG(prefix + "teaser_" + QString::number(i) + "_isos.svg");
		ExportIsosInterpolate(prefix + "teaser_" + QString::number(i) + "_inter.png", 1024, 1024);
		ExportMSEAndPBRRender(prefix + "teaser_" + QString::number(i) + "_pbr.png");

		ResampleIsos(50);
		RemoveSmallIsolines(2000);
		ProtectSmallIsolines(10000);
		SmoothIsos(300, 0.1, 15);
		CleanProtectedZones();
		SmoothIsos(300, 0.1, 10);
		CleanProtectedZones();

		ExportIsosSVG(prefix + "teaser_" + QString::number(i) + "_isos_smoothed.svg");
		ExportIsosInterpolate(prefix + "teaser_" + QString::number(i) + "_inter_smoothed.png", 1024, 1024);
		ExportMSEAndPBRRender(prefix + "teaser_" + QString::number(i) + "_pbr_smoothed.png");
	}

	// Old teaser
	//QVector<QString> masks = {
	//	"../images/input/teaser-old-1.png", // Mesas
	//	"../images/input/teaser-old-2.png", // Coastlines
	//	"../images/input/teaser-old-3.png", // RidgeLines
	//};
	//
	//QVector<HistogramD> histograms = {
	//	HistogramD(min_height, max_height, { 0.42, 0.3, 0.55, 0.89, 1.04, 0.71, 0.73, 0.6, 0.8, 0.7, 1 }),
	//	HistogramD(min_height, max_height, { 1, .73, .62, .52, .42, .41, .38, .31, .33, .19, .05 }),
	//	HistogramD(min_height, max_height, { 1, .73, .62, .52, .42, .41, .38, .31, .33, .19, .05 })
	//};
	//
	//ChangeBoxes(b);
	//ChangeEstimatedNbParticles(nb_particles);
	//ChangeHeights(min_height, max_height);
	//ChangeNoise(uniform(256, 256));
	//
	//for (int i = 0; i < masks.size(); ++i)
	//{
	//	ChangeMaskFromFile(masks[i], 3);
	//
	//	CreateGenerationZones();
	//	GenerateV3();
	//
	//	ChangeHistogram(histograms[i]);
	//
	//	ExportMask(prefix + "teaser_" + QString::number(i) + "_input.svg");
	//	ExportNoise(prefix + "teaser_" + QString::number(i) + "_noise.png");
	//	ExportHistogram(prefix + "teaser_" + QString::number(i) + "_histo.svg");
	//	ExportIsosSVG(prefix + "teaser_" + QString::number(i) + "_isos.svg");
	//
	//	if (i == 0)
	//		ExportPBRRender(prefix + "teaser_" + QString::number(i) + "_pbr.png", 256, 3, 0.3, 1);
	//	else
	//		ExportPBRRender(prefix + "teaser_" + QString::number(i) + "_pbr.png", 256, 3, 1, 0.5);
	//}
}

void MainAmplificationWindow::GenerateFigure2(const QString& prefix)
{
	QDir dir(prefix);
	if (!dir.exists())
		dir.mkpath(".");

	Box2 b(15000);
	double nb_particles = 500;
	double min_height = 0;
	double max_height = 3000;

	ChangeBoxes(b);
	ChangeHeights(min_height, max_height);
	ChangeEstimatedNbParticles(nb_particles);
	ChangeHistogram(HistogramD(min_height, max_height, { 1, 0.6, 0.8, 0.5, 0.45, 0.43, 0.29, 0.33, 0.15 }));
	ChangeNoise(fbm(256, 256, 3));
	ChangeMaskFromIsos(input_teaser_article());

	ExportMaskSVG(prefix + "pipeline_input.svg");
	ExportHistogram(prefix + "pipeline_histo.svg");
	ExportNoise(prefix + "pipeline_noise.png");

	Generate();

	ExportGraphHeights(prefix + "pipeline_graph.svg");
	ExportIsosSVG(prefix + "pipeline_isos.svg");
	ExportIsosStairs(prefix + "dem_stairs.png");

	//ChangeCamera(150, 45);
	DefaultCamera();
	ExportMSEAndPBRRender(prefix + "pipeline_reconstruction_pbr.png");
	m_hf.CreateImage().flipped().save(prefix + "dem_mse.png");

	// Edition
	//for (int i = 0; i < isos.Size(); ++i)
	//{
	//	isos[i] = isos[i].Resample(100).Smooth(300, 0.1, 10);
	//}
	//scene = isos.ToScene(true);
	//System::FlipVertical(*scene, box.GetQtRect());
	//System::SaveSvg(scene, prefix + "pipeline_edition.svg", box.GetQtRect());
}

void MainAmplificationWindow::GenerateFigure5(const QString& prefix)
{
	QDir dir(prefix);
	if (!dir.exists())
		dir.mkpath(".");

	int sample = 250;
	ArticleUtils::EdenArticle(sample, 4, prefix + "eden1.svg");
	ArticleUtils::EdenArticle(sample, 6, prefix + "eden2.svg");
	ArticleUtils::EdenArticle(sample, 37, prefix + "eden3.svg");
	ArticleUtils::EdenOnTin(sample, prefix + "eden4.svg");
}

void MainAmplificationWindow::GenerateFigure6And8(const QString& prefix)
{
	QDir dir(prefix);
	if (!dir.exists())
		dir.mkpath(".");

	int nb_isos = 10;
	int nb_particles = 150;

	IsoLines isos({ PolygonReunion().Scaled(Vector2(1, 1))});
	Box2 b = isos.GetBox().ScaledCentered(1.2);

	ChangeBoxes(b);
	ChangeEstimatedNbParticles(nb_particles);
	ChangeMaskFromIsos(isos);
	ChangeNoise(uniform(256, 256));

	CreateGenerationZones();
	GenerateV1(100, prefix);

	m_extraction_opt.graph_white_background = false;

	ChangeHistogramFromFunction(3, nb_isos);
	SetIsosFromHistogram();
	ExportGraphHeights(prefix + "graph_1.svg");
	ExportHistogram(prefix + "histo_1.svg");

	ChangeHistogramFromFunction(6, nb_isos);
	SetIsosFromHistogram();
	ExportGraphHeights(prefix + "graph_2.svg");
	ExportHistogram(prefix + "histo_2.svg");

	ChangeHistogramFromFunction(0, nb_isos);
	SetIsosFromHistogram();
	ExportGraphHeights(prefix + "graph_3.svg");
	ExportHistogram(prefix + "histo_3.svg");

}

void MainAmplificationWindow::GenerateFigure7(const QString& prefix)
{
	QDir dir(prefix);
	if (!dir.exists())
		dir.mkpath(".");

	double radius = 12.5;
	ScalarField2 m(Box2(1000, 500), QImage("../images/input/mask_2_bis.png").flipped());
	ArticleUtils::GraphConstructionDebug(m, radius, prefix);
}

void MainAmplificationWindow::GenerateFigure10(const QString& prefix)
{
	QDir dir(prefix);
	if (!dir.exists())
		dir.mkpath(".");

	int nb_isos = 12;
	IsoLines isos({ PolygonReunion().Scaled(0.01) });
	Box2 b = isos.GetBox().ScaledCentered(1.2);

	ChangeBoxes(b);
	ChangeMaskFromIsos(isos);
	ChangeHistogramFromFile("../Data/dems/reunion.png", nb_isos);

	// Noises
	QMap<QString, ScalarField2> noises = {
		{"uniform", uniform(256, 256)},
		{"fbm", fbm(256, 256, 10)}
	};

	for (QString noise_name : noises.keys())
	{
		ScalarField2 noise = noises[noise_name];
		ChangeNoise(noise);
		ExportNoise(prefix + noise_name + ".png");

		for (double nb_particles : {150, 500, 2000, 10000})
		{
			ChangeEstimatedNbParticles(nb_particles);
			Generate(1);

			ExportGraphHeights(prefix + noise_name + "_eden_" + QString::number(nb_particles) + ".svg");
			ExportIsosSVG(prefix + noise_name + "_isos_" + QString::number(nb_particles) + ".svg");
		}
	}
}

void MainAmplificationWindow::GenerateFigure11(const QString& prefix)
{
	QDir dir(prefix);
	if (!dir.exists())
		dir.mkpath(".");

	Box2 b(500);
	int nb_isos = 12;
	int nb_particles = 3000;
	
	ChangeBoxes(b);
	ChangeMaskFromFile("../images/input/mask.png");
	ChangeHistogramFromFile("../Data/dems/rockies_zoom_breach.png", nb_isos);
	ChangeEstimatedNbParticles(nb_particles);
	
	// Masque 1 zone
	ExportMaskSVG(prefix + "river_mask.svg");
	
	// User noise
	ChangeNoiseFromFile("../images/input/proba.png", 4);
	ExportNoise(prefix + "river_noise.png");
	
	Generate(1);
	
	ExportGraphHeights(prefix + "river_eden.svg");
	ExportIsosSVG(prefix + "river_isos.svg");
	
	// Uniform noise
	ChangeNoise(uniform(256, 256));
	ExportNoise(prefix + "uniform_noise.png");
	
	Generate(1);
	
	ExportGraphHeights(prefix + "uniform_eden.svg");
	ExportIsosSVG(prefix + "uniform_isos.svg");
}

void MainAmplificationWindow::GenerateFigure12(const QString& prefix)
{
	QDir dir(prefix);
	if (!dir.exists())
		dir.mkpath(".");

	Box2 b(500);
	int nb_particles = 600;

	// Masque 3 zones
	ChangeBoxes(b);
	ChangeMaskFromFile("../images/input/mask_boundaries.png", 3);
	ChangeEstimatedNbParticles(nb_particles);

	CreateGenerationZones();

	ExportMask(prefix + "double_eden_boundaries_input.png");
	ExportMaskSVG(prefix + "double_eden_boundaries_input.svg");

	ArticleUtils::DoubleEdenBoundaries(m_generation_graph_zones, prefix);
}

void MainAmplificationWindow::GenerateFigure13And14(const QString& prefix)
{
	QDir dir(prefix);
	if (!dir.exists())
		dir.mkpath(".");

	Box2 b(15000);
	int nb_isos = 12;
	int nb_particles = 3000;
	double min_height = 0;
	double max_height = 4500;

	ChangeBoxes(b);
	ChangeMaskFromFile("../images/input/multi_mask.png", 3);
	ChangeHistogramFromFile("../Data/dems/rockies_zoom_breach.png", nb_isos);
	ChangeNoise(uniform(256, 256));
	ChangeEstimatedNbParticles(nb_particles);
	ChangeHeights(min_height, max_height);

	//ChangeCamera(162, 40, 30000, Vector(5400, 5600, 3000));
	ChangeCamera(172, 25, 30000, Vector(3500, 4500, 700));
	//ChangeCamera(90, 35, 34000, Vector(6000, 500, 0));

	ExportMask(prefix + "multi_mask.png");
	ExportMaskSVG(prefix + "multi_mask.svg");

	Generate(2);

	ExportMSEAndPBRRender(prefix + "bad_multi_mask_pbr.png");
	ExportGraphHeights(prefix + "bad_multi_mask_eden.svg");
	ExportIsosSVG(prefix + "bad_multi_mask_isos.svg");

	Generate(3);

	ExportMSEAndPBRRender(prefix + "multi_mask_pbr.png");
	ExportGraphHeights(prefix + "multi_mask_eden.svg");
	ExportIsosSVG(prefix + "multi_mask_isos.svg");
	ExportGraphZones(prefix + "multi_mask_zones.svg");
}

void MainAmplificationWindow::GenerateFigure16And19And20(const QString& prefix)
{
	QDir dir(prefix);
	if (!dir.exists())
		dir.mkpath(".");

	Box2 b(17000, 17000);
	Box2 zoomb(Vector2(0, 2000), 3000);
	QString sf = "../Data/dems/rockies_zoom_breach.png";
	int nb_isos = 20;
	int nb_particles = 5000;

	ChangeBoxes(b);
	ChangeEstimatedNbParticles(nb_particles);
	ChangeMaskFromFile(sf, nb_isos);
	ChangeHistogramFromFile(sf, nb_isos);
	ChangeNoise(uniform(256, 256));

	CreateGenerationZones();
	GenerateV3();
	SetIsosFromHistogram();

	ChangeBoxes(zoomb);
	RemoveSmallIsos(1000);
	ExportIsosSVG(prefix + "before_edit.svg");

	ResampleIsos(50);
	SmoothIsos(250, 0.1, 10);

	ExportIsosSVG(prefix + "after_smooth.svg");

	IsoLines copy = m_isos;

	{
		QVector<Vector2> path = {
			Vector2(-3276.21 ,  734.322),
			Vector2(-3276.21 ,  748.444),
			Vector2(-3276.21 ,  762.565),
			Vector2(-3290.33 ,  790.808),
			Vector2(-3290.33 ,  847.295),
			Vector2(-3290.33 ,  889.659),
			Vector2(-3290.33 ,  932.024),
			Vector2(-3276.21 ,  988.511),
			Vector2(-3262.08 ,  1045),
			Vector2(-3233.84 ,  1087.36),
			Vector2(-3191.48 ,  1143.85),
			Vector2(-3149.11 ,  1200.33),
			Vector2(-3149.11 ,  1214.46),
			Vector2(-3092.63 ,  1256.82),
			Vector2(-3036.14 ,  1285.06),
			Vector2(-2979.65 ,  1313.31),
			Vector2(-2909.05 ,  1355.67),
			Vector2(-2838.44 ,  1383.91),
			Vector2(-2739.59 ,  1398.04),
			Vector2(-2668.98 ,  1412.16),
			Vector2(-2654.86 ,  1412.16),
			Vector2(-2570.13 ,  1440.4),
			Vector2(-2471.28 ,  1454.52),
			Vector2(-2400.67 ,  1468.64),
			Vector2(-2330.06 ,  1468.64),
			Vector2(-2259.45 ,  1482.77),
			Vector2(-2188.84 ,  1482.77),
			Vector2(-2132.36 ,  1496.89),
			Vector2(-2075.87 ,  1496.89),
			Vector2(-2005.26 ,  1511.01),
			Vector2(-1934.66 ,  1525.13),
			Vector2(-1864.05 ,  1525.13),
			Vector2(-1793.44 ,  1539.25),
			Vector2(-1722.83 ,  1553.37),
			Vector2(-1652.22 ,  1553.37),
			Vector2(-1581.62 ,  1567.5),
			Vector2(-1511.01 ,  1567.5),
			Vector2(-1454.52 ,  1567.5),
			Vector2(-1383.91 ,  1581.62),
			Vector2(-1299.19 ,  1595.74),
			Vector2(-1214.46 ,  1609.86),
			Vector2(-1143.85 ,  1623.98),
			Vector2(-1073.24 ,  1638.1),
			Vector2(-1016.75 ,  1652.22),
			Vector2(-946.146 ,  1680.47),
			Vector2(-875.538 ,  1708.71),
			Vector2(-819.052 ,  1736.95),
			Vector2(-762.565 ,  1779.32),
			Vector2(-706.079 ,  1807.56),
			Vector2(-635.471 ,  1849.93),
			Vector2(-578.985 ,  1892.29),
			Vector2(-522.498 ,  1920.53),
			Vector2(-466.012 ,  1962.9),
			Vector2(-395.404 ,  2019.39),
			Vector2(-338.918 ,  2061.75),
			Vector2(-282.432 ,  2118.24),
			Vector2(-225.945 ,  2174.72),
			Vector2(-169.459 ,  2217.09),
			Vector2(-112.973 ,  2259.45),
			Vector2(-70.6079 ,  2301.82),
			Vector2(-28.2432 ,  2344.18),
			Vector2(0 ,  2386.55),
			Vector2(42.3647 ,  2428.91),
			Vector2(70.6079 ,  2471.28),
			Vector2(112.973 ,  2527.76),
			Vector2(155.337 ,  2570.13),
			Vector2(197.702 ,  2612.49),
			Vector2(240.067 ,  2668.98),
			Vector2(282.432 ,  2697.22),
			Vector2(296.553 ,  2739.59),
			Vector2(324.796 ,  2767.83),
			Vector2(353.039 ,  2781.95),
			Vector2(367.161 ,  2810.19),
		};
		double radius = 500;
		double mu = 0.90;

		Warp(path, radius, mu);
	}
	{
		QVector<Vector2> path = {
			Vector2(880.593 ,  2557.91),
			Vector2(888.98 ,  2557.91),
			Vector2(897.366 ,  2557.91),
			Vector2(897.366 ,  2541.14),
			Vector2(897.366 ,  2532.75),
			Vector2(897.366 ,  2515.98),
			Vector2(897.366 ,  2499.21),
			Vector2(905.753 ,  2490.82),
			Vector2(905.753 ,  2465.66),
			Vector2(905.753 ,  2440.5),
			Vector2(914.139 ,  2415.34),
			Vector2(922.526 ,  2390.18),
			Vector2(930.912 ,  2356.63),
			Vector2(939.299 ,  2323.09),
			Vector2(956.072 ,  2297.93),
			Vector2(964.459 ,  2264.38),
			Vector2(972.845 ,  2239.22),
			Vector2(989.619 ,  2205.68),
			Vector2(1006.39 ,  2180.52),
			Vector2(1023.17 ,  2146.97),
			Vector2(1039.94 ,  2113.42),
			Vector2(1065.1 ,  2079.88),
			Vector2(1081.87 ,  2046.33),
			Vector2(1107.03 ,  2012.78),
			Vector2(1123.8 ,  1987.62),
			Vector2(1132.19 ,  1987.62),
			Vector2(1140.58 ,  1962.46),
			Vector2(1165.74 ,  1945.69),
			Vector2(1165.74 ,  1937.3),
			Vector2(1182.51 ,  1920.53),
			Vector2(1207.67 ,  1886.98),
			Vector2(1232.83 ,  1870.21),
			Vector2(1249.6 ,  1853.44),
			Vector2(1274.76 ,  1828.28),
			Vector2(1291.54 ,  1811.51),
			Vector2(1316.7 ,  1786.35),
			Vector2(1341.86 ,  1769.57),
			Vector2(1367.02 ,  1744.41),
			Vector2(1392.18 ,  1727.64),
			Vector2(1408.95 ,  1710.87),
			Vector2(1417.34 ,  1710.87),
			Vector2(1434.11 ,  1694.09),
			Vector2(1450.88 ,  1677.32),
			Vector2(1476.04 ,  1660.55),
			Vector2(1492.81 ,  1643.77),
			Vector2(1501.2 ,  1635.39),
			Vector2(1501.2 ,  1627),
			Vector2(1517.97 ,  1610.23),
			Vector2(1534.75 ,  1601.84),
			Vector2(1551.52 ,  1576.68),
			Vector2(1568.29 ,  1568.29),
			Vector2(1585.07 ,  1551.52),
			Vector2(1593.45 ,  1534.75),
			Vector2(1601.84 ,  1517.97),
			Vector2(1618.61 ,  1492.81),
			Vector2(1627 ,  1467.65),
			Vector2(1643.77 ,  1459.27),
			Vector2(1652.16 ,  1434.11),
			Vector2(1668.93 ,  1417.34),
			Vector2(1685.71 ,  1392.18),
			Vector2(1694.09 ,  1358.63),
			Vector2(1702.48 ,  1333.47),
			Vector2(1710.87 ,  1299.92),
			Vector2(1719.25 ,  1274.76),
			Vector2(1736.03 ,  1249.6),
			Vector2(1752.8 ,  1241.22),
			Vector2(1761.19 ,  1216.06),
			Vector2(1769.57 ,  1207.67),
			Vector2(1777.96 ,  1190.9),
			Vector2(1786.35 ,  1182.51),
			Vector2(1794.73 ,  1165.74),
			Vector2(1811.51 ,  1140.58),
			Vector2(1828.28 ,  1140.58),
		};
		double radius = 250;
		double mu = 0.70;

		Warp(path, radius, mu);
	}
	{
		QVector<Vector2> path = {
			Vector2(2910.15 ,  3312.71),
			Vector2(2901.76 ,  3312.71),
			Vector2(2868.22 ,  3321.09),
			Vector2(2843.06 ,  3329.48),
			Vector2(2801.12 ,  3354.64),
			Vector2(2742.42 ,  3371.41),
			Vector2(2700.48 ,  3388.19),
			Vector2(2666.94 ,  3396.57),
			Vector2(2616.62 ,  3404.96),
			Vector2(2549.53 ,  3421.73),
			Vector2(2482.43 ,  3430.12),
			Vector2(2432.11 ,  3438.51),
			Vector2(2390.18 ,  3446.89),
			Vector2(2331.47 ,  3446.89),
			Vector2(2323.09 ,  3446.89),
			Vector2(2256 ,  3446.89),
			Vector2(2188.9 ,  3446.89),
			Vector2(2105.04 ,  3446.89),
			Vector2(2021.17 ,  3438.51),
			Vector2(1945.69 ,  3438.51),
			Vector2(1870.21 ,  3430.12),
			Vector2(1786.35 ,  3421.73),
			Vector2(1702.48 ,  3404.96),
			Vector2(1627 ,  3388.19),
			Vector2(1618.61 ,  3388.19),
			Vector2(1526.36 ,  3379.8),
			Vector2(1434.11 ,  3363.03),
			Vector2(1350.24 ,  3346.25),
			Vector2(1266.38 ,  3337.87),
			Vector2(1199.28 ,  3329.48),
			Vector2(1132.19 ,  3329.48),
			Vector2(1081.87 ,  3321.09),
			Vector2(1014.78 ,  3321.09),
			Vector2(956.072 ,  3312.71),
			Vector2(897.366 ,  3312.71),
			Vector2(847.047 ,  3312.71),
			Vector2(796.727 ,  3312.71),
			Vector2(754.794 ,  3312.71),
			Vector2(712.861 ,  3321.09),
			Vector2(679.315 ,  3329.48),
			Vector2(654.155 ,  3337.87),
			Vector2(620.608 ,  3346.25),
			Vector2(587.062 ,  3363.03),
			Vector2(561.902 ,  3371.41),
			Vector2(545.129 ,  3379.8),
			Vector2(545.129 ,  3388.19),
			Vector2(519.969 ,  3396.57),
			Vector2(494.809 ,  3404.96),
			Vector2(478.036 ,  3413.35),
			Vector2(461.263 ,  3430.12),
			Vector2(444.49 ,  3438.51),
			Vector2(427.717 ,  3446.89),
			Vector2(419.33 ,  3455.28),
			Vector2(410.943 ,  3463.67),
			Vector2(410.943 ,  3472.05),
		};
		double radius = 250;
		double mu = 0.80;

		Warp(path, radius, mu);
	}

	ExportIsosSVG(prefix + "after_warp.svg");
	SetIsos(copy, false, true);

	{
		QVector<Vector2> path = {
		   Vector2(-2631.23, 5023.26),
		   //Vector2(-2531.56, 4903.65),
		   //Vector2(-2093.02, 4504.98),
		   //Vector2(-1694.35, 4126.25),
		   //Vector2(-1375.42, 3787.38),
		   //Vector2(-1056.48, 3368.77),
		   //Vector2(-936.877, 3129.57),
		   //Vector2(-797.342, 2850.5),
		   //Vector2(-777.409, 2850.5),
		   //Vector2(-697.674, 2651.16),
		   //Vector2(-538.206, 2392.03),
		   //Vector2(-398.671, 2272.43),
		   //Vector2(-259.136, 2112.96),
		   //Vector2(-79.7342, 1933.55),
		   //Vector2(0, 1833.89),
		   //Vector2(59.8007, 1734.22),
		   //Vector2(119.601, 1694.35),
		   Vector2(159.468, 1654.49),
		};

		double radius = 1500;
		double strength = -0.7;

		DisplacementAlongCurve(path, radius, strength);
	}
	{
		QVector<Vector2> path = {
			Vector2(558.621, 4365.52),
			Vector2(579.31, 4365.52),
			Vector2(765.517, 4448.28),
			Vector2(1055.17, 4655.17),
			Vector2(1200, 4696.55),
			Vector2(1324.14, 4717.24),
			Vector2(1468.97, 4634.48),
			Vector2(1593.1, 4510.34),
			Vector2(1696.55, 4344.83),
			Vector2(1800, 4117.24),
			Vector2(1841.38, 3848.28),
			Vector2(1862.07, 3579.31),
			Vector2(1862.07, 3227.59),
			Vector2(1882.76, 3041.38),
			Vector2(1924.14, 2855.17),
			Vector2(1944.83, 2668.97),
			Vector2(2006.9, 2462.07),
			Vector2(2048.28, 2296.55),
			Vector2(2089.66, 2151.72),
			Vector2(2110.34, 1965.52),
			Vector2(2110.34, 1675.86),
			Vector2(2110.34, 1489.66),
			Vector2(2151.72, 1324.14),
		};

		double radius = 1500;
		double strength = 0.5;

		DisplacementAlongCurve(path, radius, strength);
	}

	ExportIsosSVG(prefix + "after_sloping.svg");
	SetIsos(copy, false, true);
}

void MainAmplificationWindow::GenerateFigure21(const QString& prefix)
{
	QDir dir(prefix);
	if (!dir.exists())
		dir.mkpath(".");

	Box2 b(15000);
	double min_height = 0;
	double max_height = 2500;
	int nb_isos = 12;
	int nb_particles = 10000;

	QVector<QString> masks =
	{
		"../images/input/multi_mask_2.png",
		//"../images/input/mask_lambda.png",
		//"../images/input/mask_desert.png",
		//"../images/input/mask_triple.png",
	};

	QVector<QString> histos =
	{
		"../Data/dems/rockies_zoom_breach.png",
		//3,
		//"../Data/dems/rockies_zoom_breach.png",
		//"../Data/dems/reunion.png",
	};

	ChangeBoxes(b);
	ChangeHeights(min_height, max_height);
	ChangeEstimatedNbParticles(nb_particles);
	ChangeNoise(uniform(256, 256));

	for (int i = 0; i < masks.size(); ++i)
	{
		ChangeMaskFromMaskFile(masks[i]);

		CreateGenerationZones();
		GenerateV3();

		ChangeHistogramFromFile(histos[i], nb_isos);

		ResampleIsos(50);
		ProtectSmallIsolines(1000);
		SmoothIsos(300, 0.1, 10);
		CleanProtectedZones();

		ExportMaskSVG(prefix + "result_" + QString::number(i) + "_input.svg");
		ExportHistogram(prefix + "result_" + QString::number(i) + "_histo.svg");
		ExportIsosSVG(prefix + "result_" + QString::number(i) + "_isos.svg");
		ExportIsosSmoothStairs(prefix + "result_" + QString::number(i) + "_stairs.png", 1024, 1024);
		ExportIsosInterpolate(prefix + "result_" + QString::number(i) + "_interpolate.png", 1024, 1024);
		ExportIsosInterpolate(prefix + "result_" + QString::number(i) + "_interpolate256.png", 256, 256);

		// Already created terrains
		ScalarField2 sf1(b, QImage("../Data/ampli/deep_0" + QString::number(i) + "_01.png"), min_height, max_height);
		sf1.CreateImage().save(prefix + "result_" + QString::number(i) + "_deep.png");

		ScalarField2 sf2(b, QImage("../Data/ampli/mse_0" + QString::number(i) + "_01.png"), min_height, max_height);
		sf2.CreateImage().save(prefix + "result_" + QString::number(i) + "_mse.png");

		ScalarField2 sf3(b, QImage("../Data/ampli/sparse_0" + QString::number(i) + "_01.png"), min_height, max_height);
		sf3.CreateImage().save(prefix + "result_" + QString::number(i) + "_sparse.png");
	}
}

void MainAmplificationWindow::GenerateFigure22(const QString& prefix)
{
	QDir dir(prefix);
	if (!dir.exists())
		dir.mkpath(".");

	Box2 b(15000);
	int nb_isos = 12;

	QVector<QString> sfs =
	{
		//"../Data/dems/reunion.png",
		//"../Data/dems/corse.png",
		//"../Data/dems/olympus-513.png",
		"../Data/dems/vesuve.png",
		"../Data/dems/rockies_zoom_breach.png",
		//"../Data/dems/colorado.png",
		"../Data/dems/alps.png",
		"../Data/dems/ruapehu.png",
		"../images/input/mask_lambda.png",
		"../images/input/mask_desert.png",
		"../images/input/mask_triple.png",
		"../images/input/multi_mask_2.png",
	};

	QVector<variant<int, QString>> histos =
	{
		sfs[0],
		sfs[1],
		sfs[2],
		sfs[3],
		//sfs[4],
		//sfs[5],
		//sfs[6],
		//sfs[7],
		3,
		"../Data/dems/rockies_zoom_breach.png",
		"../Data/dems/reunion.png",
		"../Data/dems/rockies_zoom_breach.png"
	};

	QVector<int> nb_zones =
	{
		/*3, 3, 3,*/ 3, 3, /*3,*/ 3, 3, 2, 3, 3, 3
	};

	ExtractionOpt opt;
	opt.extract_pbr = false;
	opt.extract_stairs = false;
	ChangeExportOptions(opt);

	ChangeBoxes(b);
	ChangeEstimatedNbParticles(10000);
	ChangeNoise(uniform(256, 256));

	for (int i = 0; i < sfs.size(); ++i)
	{
		ChangeMaskFromFile(sfs[i], nb_zones[i], 0.01);

		CreateGenerationZones();
		GenerateV3();
	
		if (holds_alternative<int>(histos[i]))
			ChangeHistogramFromFunction(get<int>(histos[i]), nb_isos);
		else
			ChangeHistogramFromFile(get<QString>(histos[i]), nb_isos);
	
		ExportAll(prefix + QString::number(i));
	}

	DefaultExportOptions();

	// comparisons
	//for (int i = 0; i < sfs.size(); ++i)
	//{
	//	ChangeMaskFromFile(sfs[i], nb_zones[i], 0.01);
	//	IsoLines isos = isolines_mask(m_generation_mask);
	//	SetIsos(isos);
	//	ExportIsosSVG(prefix + "verif_" + QString::number(i) + "_isos.svg");
	//	ExportIsosInterpolate(prefix + "verif_" + QString::number(i) + "_inter.png", 256, 256);
	//}
}

void MainAmplificationWindow::GenerateFigure23(const QString& prefix)
{
	QDir dir(prefix);
	if (!dir.exists())
		dir.mkpath(".");

	IsoLinePoly initial_contour = PolygonHawaii();
	ScalarField2 mask = polygon_mask(initial_contour, 1024, 1024);

	int nb_particles = 10000;
	int nb_isos = 12;

	QVector<variant<ScalarField2, QString>> probas
	{
		uniform(256, 256),
		fbm(256, 256, 3),
		"../Images/input/hawaii_rivers.png",
	};

	get<ScalarField2>(probas[1]).Pow(2);

	QVector<int> histograms_id
	{
		3, 4, 7,
	};

	ChangeBoxes(mask.GetBox());
	ChangeMask(mask);
	ChangeEstimatedNbParticles(nb_particles);

	ExportMaskSVG(prefix + "parameters_mask.svg");

	for (int i = 0; i < histograms_id.size(); ++i)
	{
		ChangeHistogramFromFunction(histograms_id[i], nb_isos);
		ExportHistogram(prefix + "parameters_histo_" + QString::number(i) + ".svg");
	}

	CreateGenerationZones();
	//ExportGraphZones(prefix + "parameters_zones.svg");

	for (int i = 0; i < probas.size(); ++i)
	{
		if (holds_alternative<ScalarField2>(probas[i]))
			ChangeNoise(get<ScalarField2>(probas[i]));
		else
			ChangeNoiseFromFile(get<QString>(probas[i]));

		ExportNoise(prefix + "parameters_noise_" + QString::number(i) + ".png");
		GenerateV3();

		for (int j = 0; j < histograms_id.size(); ++j)
		{
			ChangeHistogramFromFunction(histograms_id[j], nb_isos);

			ExportIsosSVG(prefix + "parameters_isos_" + QString::number(i) + "_" + QString::number(j) + ".svg");
			ExportIsosInterpolate(prefix + "parameters_inter_" + QString::number(i) + "_" + QString::number(j) + ".png", 1024, 1024);

			m_isos.Resample(50);
			m_isos.Smooth(300, 0.1, 20);
			SetIsos(m_isos);

			ExportIsosSmoothStairs(prefix + "parameters_stairs_" + QString::number(i) + "_" + QString::number(j) + ".png", 1024, 1024);
		}
	}
}

void MainAmplificationWindow::GenerateFigure24(const QString& prefix)
{
	QDir dir(prefix);
	if (!dir.exists())
		dir.mkpath(".");

	Box2 b(15000);
	int nb_isos = 12;
	int nb_particles = 10000;
	int min_height = 0;
	int max_height = 5000;
	QString sf = "../Data/dems/alps.png";

	QVector<int> smooths = { 20, 100, 300 };

	ChangeBoxes(b);
	ChangeHistogramFromFile(sf, nb_isos);
	ChangeEstimatedNbParticles(nb_particles);
	ChangeNoise(fbm(256, 256, 10), 3);
	//ChangeNoise(uniform(256, 256));
	ChangeHeights(min_height, max_height);
	LookDownCamera();

	for (int i = 0; i < smooths.size(); ++i)
	{
		// Decomment for same area regions
		//QSet<double> heights;
		//heights.insert(20);
		//heights.insert(500);
		//heights.insert(2000);

		//HeightField real_middle(b, QImage(sf).scaled(512, 512, Qt::KeepAspectRatio, Qt::FastTransformation).flipped(), min_height, max_height);
		//real_middle.Smooth(smooths[i]);
		//real_middle.SetRange(min_height, max_height);

		//IsoLines iso_mask(real_middle, heights);
		//ChangeMaskFromIsos(iso_mask);

		// Same heights  difference between bins
		ChangeMaskFromFile(sf, 3, 0.01, smooths[i]);

		CreateGenerationZones();
		GenerateV3();
		SetIsosFromHistogram();

		ExportMaskSVG(prefix + "mask_" + QString::number(i) + ".svg");
		ExportMSEAndPBRRender(prefix + "pbr_" + QString::number(i) + ".png");
		ExportIsosSVG(prefix + "isos_" + QString::number(i) + ".svg");
		ExportIsosInterpolate(prefix + "inter_" + QString::number(i) + ".png", 1024, 1024);
		ExportIsosSmoothStairs(prefix + "stairs_" + QString::number(i) + ".png", 1024, 1024);
	}

	ExportNoise(prefix + "noise.png");
	ExportHistogram(prefix + "histogram.svg");
}

void MainAmplificationWindow::GenerateFigure25AndTable1(const QString& prefix)
{
	QDir dir(prefix);
	if (!dir.exists())
		dir.mkpath(".");

	Box2 b(15000);
	QString maskFile = "../Data/dems/reunion.png";
	//QString maskFile = "../images/input/circles2.png";
	int nb_tests = 20;
	//int nb_tests = 1;

	ChangeBoxes(b);
	ChangeNoise(uniform(256, 256));

	QString debug = "";
	ChangeMaskFromFile(maskFile, 1);
	ExportMaskSVG(prefix + "mask.svg");

	auto t = [&](const QVector<qint64> timings)
		{
			double mean = 0;
			double var = 0;
			for (qint64 t : timings)
				mean += t;
			mean /= timings.size();
			for (qint64 t : timings)
				var += (t - mean) * (t - mean);
			var /= timings.size();
			double std = sqrt(var);

			QString s = "";
			s += "MEAN: " + QString::number(mean) + ", ";
			s += "VAR: " + QString::number(var) + ", ";
			s += "STD: " + QString::number(std) + ", ";
			s += "VALUES: ";
			for (qint64 t : timings)
				s += QString::number(t) + " ";
			return s;
		};

	// Note for the time taken for multiple zone:
	// 90% of time taken is due to the method DoubleEdenNodesAssignments (this percentage increase with N since this is the only non O(N) part of the algo)
	// This method run two eden by zone except when there is no upper boundary
	// This is why the runtime for one zone is twice less than with multiple zone
	//	- when the number of zone tends to infinity -> twice longer than one zone
	//  - in our example, the last zone often contains way less node, so even with only two zones we already atteign the maximum
	//  - the time taken is in fact O(N*B) where B is the border size. But this size change often. We could say this is O(sqrt(N)), but it highly depends on the mask and proba field
	//  - so the time taken can change according to this border

	// Time taken with one zone with different number of extracted isolines
	auto f = [&](int nb_particles)
		{
			ChangeEstimatedNbParticles(nb_particles);

			QElapsedTimer timer;
			timer.start();

			QVector<qint64> times;
			for (int i = 0; i < nb_tests; ++i)
			{
				CreateGenerationZones();

				qint64 t1 = timer.elapsed();

				//GenerateV3();
				GenerateV3Endo();

				qint64 t2 = timer.elapsed();

				times.append(t2 - t1);
			}

			debug += QString::number(m_generation_graph_zones.Size()) + ":\n\tTg: " + t(times);

			for (int nb_isos : {10, 20, 30})
			{
				times.clear();
				double a, b;
				m_generation_graph_result.GetRange(a, b);

				for (int i = 0; i < nb_tests; ++i)
				{
					// We do this since it is closer to the reality of time taken
					qint64 t31 = timer.elapsed();

					for (int i = 0; i < nb_isos; ++i)
					{
						double h = Math::Lerp(a, b, (i / (double)nb_isos));
						Polygons2 polys = m_generation_graph_result.ContourLines(h);
					}

					qint64 t32 = timer.elapsed();

					times.append(t32 - t31);
				}
				debug += "\n\tT" + QString::number(nb_isos) + ": " + t(times);
			}

			debug += "\n\n";
		};

	// Time taken with multiple zones with same amount of extracted isolines
	auto g = [&](int nb_particles)
		{
			ChangeEstimatedNbParticles(nb_particles);
			//CreateGenerationZones();

			QElapsedTimer timer;
			timer.start();

			debug += QString::number(nb_particles);

			for (int nb_isos_mask : {1, 2, 3, 4, 5, 6})
			{
				ChangeMaskFromFile(maskFile, nb_isos_mask);
				ExportMaskSVG(prefix + "mask_" + QString::number(nb_isos_mask) + ".svg");

				QVector<qint64> times;
				for (int i = 0; i < nb_tests; ++i)
				{
					CreateGenerationZones();

					qint64 t1 = timer.elapsed();

					//GenerateV3();
					GenerateV3Endo();

					qint64 t2 = timer.elapsed();

					times.append(t2 - t1);
				}

				debug += "\n\t" + QString::number(nb_isos_mask) + " regions: " + t(times);
			}

			debug += "\n\n";
		};

	debug += "===== ONE REGION =====\n";
	for (int nb_particles : {200000, 250000, 300000})//{1000, 10000, 20000, 50000, 100000, 500000})
	{
		f(nb_particles);
		cout << debug.toStdString();
	}

	debug += "\n===== MULTIPLE REGIONS =====\n";
	for (int nb_particles : {200000, 250000, 300000})//{1000, 10000, 20000, 50000, 100000, 500000})
	{
		g(nb_particles);
		cout << debug.toStdString();
	}

	debug += +"\n";

	QFile file(prefix + "time.txt");
	if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QTextStream out(&file);
		out << debug;
		file.close();
	}
}

void MainAmplificationWindow::GenerateFigure26(const QString& prefix)
{
	QDir dir(prefix);
	if (!dir.exists())
		dir.mkpath(".");

	Box2 b(15000);
	int nb_isos = 12;
	int nb_particles = 10000;
	double min_height = 0;
	double max_height = 5000;
	QString mask = "../images/input/multi_mask_2.png";
	QString histo = "../Data/dems/rockies_zoom_breach.png";

	ChangeBoxes(b);
	ChangeHeights(min_height, max_height);
	ChangeEstimatedNbParticles(nb_particles);
	ChangeNoise(uniform(256, 256));
	ChangeHistogramFromFile(histo, nb_isos);
	ChangeMaskFromMaskFile(mask);
	ChangeCamera(200, 42, 60000);

	CreateGenerationZones();
	GenerateV3();
	SetIsosFromHistogram();

	ExportHistogram(prefix + "histogram.svg");
	ExportMaskSVG(prefix + "mask.svg");
	ExportMSEAndPBRRender(prefix + "pbr.png");
	ExportIsosSVG(prefix + "isos.svg");
	ExportIsosInterpolate(prefix + "inter.png", 1024, 1024);
	ExportIsosSmoothStairs(prefix + "stairs.png", 1024, 1024);

	// We change heights to have exactly the same heights in every pbr fields.
	m_hf.GetRange(min_height, max_height);

	// gradient terrain authoring
	HeightField hf(b, QImage("../Data/gradient/result_c.png").flipped(), min_height, max_height);
	hf.CreateImage().flipped().save(prefix + "gradient.png");

	hf.DownSample(2);
	hf.ErosionAmpli(3, 1, 1);
	hf.SetRange(min_height, max_height); // pour que les deux mses aient les memes hauteurs finales
	hf.CreateImage().flipped().save(prefix + "gradient_mse.png");

	m_hf = hf;
	UpdateGeometry();
	RenderOpt opt = render_basic_opt(m_isos, m_hf);
	ExportPBRRender(opt, prefix + "gradient_pbr.png");

	// orometry
	hf = HeightField(b, QImage("../Data/orometry/orometry_pyrenees-histo_rockies.png").flipped(), min_height, max_height);
	hf.CreateImage().flipped().save(prefix + "orometry.png");
	hf.SetRange(min_height, max_height); // parce que les couleurs vont pas jusque 1
	hf.DownSample(3);
	hf.ErosionAmpli(3, 1, 1);
	hf.SetRange(min_height, max_height); // pour que les deux mses aient les memes hauteurs finales
	hf.CreateImage().flipped().save(prefix + "orometry_mse.png");

	m_hf = hf;
	UpdateGeometry();
	opt = render_basic_opt(m_isos, m_hf);
	ExportPBRRender(opt, prefix + "orometry_pbr.png");
}

void MainAmplificationWindow::GenerateFigure27(const QString& prefix)
{
	QDir dir(prefix);
	if (!dir.exists())
		dir.mkpath(".");

	double min_height = 0;
	double max_height = 7000;
	int nb_particles = 10000;
	Box2 b(15000);

	QVector<QString> masks = {
		"../images/input/editing-1.png",
		"../images/input/editing-2.png",
		"../images/input/editing-3.png",
	};

	QString histo1file = "../Data/dems/reunion.png";
	int histo1isos = 12;
	QString histo2file = "../Data/dems/rockies_zoom_breach.png";
	int histo2isos = 20;

	ChangeBoxes(b);
	ChangeEstimatedNbParticles(nb_particles);
	ChangeHeights(min_height, max_height);
	ChangeNoise(uniform(256, 256));
	ChangeHistogramFromFile(histo1file, histo1isos);

	ExtractionOpt opt;
	opt.extract_stairs = false;
	opt.extract_smoothed_stairs = false;
	opt.extract_interpolate = false;
	opt.interpolate_x = 1024;
	opt.interpolate_y = 1024;
	opt.smoothed_stairs_x = 1024;
	opt.smoothed_stairs_y = 1024;
	opt.histogram_palette = new AnalyticPalette(3);
	ChangeExportOptions(opt);

	for (int i = 0; i < masks.size(); ++i)
	{
		ChangeMaskFromMaskFile(masks[i]);

		CreateGenerationZones();
		GenerateV3();
		SetIsosFromHistogram();

		ChangeCamera(120, 40);
		ExportAll(prefix + QString::number(i));
	}

	ChangeHistogramFromFile(histo2file, histo2isos);
	ExportAll(prefix + "edit_1_histo");

	ResampleIsos(150);
	RemoveSmallIsolines(2000);
	ProtectSmallIsolines(10000);
	SmoothIsos(400, 0.1, 15);
	CleanProtectedZones();
	SmoothIsos(400, 0.1, 10);
	CleanProtectedZones();
	ExportAll(prefix + "edit_2_smooth");

	m_uiw.export_path_edit->setText(prefix + "edit_3_warp");

	// FAIRE LE RESTE DE L'ÉDITION À LA MAIN

	/*
	// Smoothing des isos
	for (IsoLinePoly& ilp : isos)
	{
		ilp = ilp.Resample(50);
		if (ilp.Length() < 5000)
			ilp = ilp.Smooth(50, 0.1, 5);
		else
			ilp = ilp.Smooth(300, 0.1, 10);
	}
	SetIsos(isos, false, true);

	s = isos.ToScene(true);
	System::FlipVertical(*s, b.GetQtRect());
	System::SaveSvg(s, prefix + "isos_edit_2_smooth.svg", b.GetQtRect());

	ilt = IsoLineTerrain(isos);
	m_hf = ilt.SmoothStairsField(b, 1024, 1024, 1);
	m_hf.CreateImage().flipped().save(prefix + "stairs_edit_2_smooth.png");

	ExportPBRRender(prefix + "pbr_edit_2_smooth.png");
	m_hf.CreateImage().flipped().save(prefix + "mse_edit_2_smooth.png");

	// Création rivière
	QVector<Vector2> path1 = {
		Vector2(-9069.77, -2491.69),
		Vector2(-9069.77, -2591.36),
		Vector2(-9069.77, -2691.03),
		Vector2(-9169.44, -3089.7),
		Vector2(-9269.1, -3388.7),
		Vector2(-9269.1, -3787.38),
		Vector2(-9169.44, -4385.38),
		Vector2(-9169.44, -4784.05),
		Vector2(-8970.1, -5182.72),
		Vector2(-8671.1, -5581.4),
		Vector2(-8272.43, -5980.07),
		Vector2(-8172.76, -6179.4),
		Vector2(-8073.09, -6279.07),
		Vector2(-7973.42, -6478.41),
		Vector2(-7774.09, -6478.41),
		Vector2(-7574.75, -6578.07),
		Vector2(-7475.08, -6777.41),
		Vector2(-7275.75, -6877.08),
		Vector2(-7176.08, -6976.74),
		Vector2(-7076.41, -6976.74),
		Vector2(-6976.74, -7076.41),
		Vector2(-6976.74, -7176.08),
		Vector2(-6976.74, -7275.75),
		Vector2(-6677.74, -7475.08),
		Vector2(-6478.41, -7774.09),
		Vector2(-6279.07, -8073.09),
		Vector2(-5980.07, -8272.43),
		Vector2(-5780.73, -8571.43),
		Vector2(-5481.73, -8770.76),
		Vector2(-5282.39, -8970.1),
		Vector2(-4983.39, -9269.1),
		Vector2(-4883.72, -9568.11),
		Vector2(-4784.05, -9767.44),
		Vector2(-4684.39, -9966.78),
		Vector2(-4684.39, -10166.1),
		Vector2(-4684.39, -10365.4),
		Vector2(-4584.72, -10664.5),
		Vector2(-4584.72, -10764.1),
		Vector2(-4584.72, -10863.8),
		Vector2(-4584.72, -10963.5),
		Vector2(-4584.72, -11162.8),
		Vector2(-4584.72, -11362.1),
		Vector2(-4485.05, -11461.8),
	};
	QVector<Vector2> path2 = {
		Vector2(-8970.1, -697.674),
		Vector2(-9069.77, -797.342),
		Vector2(-9069.77, -897.01),
		Vector2(-9169.44, -1295.68),
		Vector2(-9269.1, -1794.02),
		Vector2(-9269.1, -2392.03),
		Vector2(-9169.44, -3089.7),
		Vector2(-8571.43, -3986.71),
		Vector2(-7873.75, -4784.05),
		Vector2(-7275.75, -5382.06),
		Vector2(-6677.74, -5880.4),
		Vector2(-6179.4, -6179.4),
		Vector2(-5581.4, -6478.41),
		Vector2(-4883.72, -6777.41),
		Vector2(-4385.38, -7176.08),
		Vector2(-3887.04, -7574.75),
		Vector2(-3588.04, -7873.75),
		Vector2(-3388.7, -8073.09),
		Vector2(-3289.04, -8372.09),
		Vector2(-3289.04, -8571.43),
		Vector2(-3289.04, -8770.76),
		Vector2(-3289.04, -8970.1),
		Vector2(-3289.04, -9069.77),
		Vector2(-3289.04, -9169.44),
		Vector2(-3289.04, -9269.1),
		Vector2(-3289.04, -9368.77),
	};
	QVector<Vector2> path3 = {
		Vector2(-7275.75, -4584.72),
		Vector2(-6976.74, -5083.06),
		Vector2(-6378.74, -5880.4),
		Vector2(-5681.06, -6578.07),
		Vector2(-4883.72, -7176.08),
		Vector2(-4385.38, -7774.09),
		Vector2(-4086.38, -8272.43),
		Vector2(-3787.38, -8870.43),
		Vector2(-3687.71, -9468.44),
		Vector2(-3687.71, -10166.1),
		Vector2(-3687.71, -10664.5),
		Vector2(-3687.71, -10963.5),
		Vector2(-3687.71, -11063.1),
		Vector2(-3787.38, -11063.1),
	};
	QVector<Vector2> path4 = {
		Vector2(-5382.06, -7275.75),
		Vector2(-5382.06, -7375.42),
		Vector2(-5382.06, -7475.08),
		Vector2(-5083.06, -7973.42),
		Vector2(-4883.72, -8471.76),
		Vector2(-4684.39, -9169.44),
		Vector2(-4584.72, -9568.11),
		Vector2(-4584.72, -10066.4),
		Vector2(-4584.72, -10465.1),
		Vector2(-4584.72, -10764.1),
		Vector2(-4684.39, -11063.1),
		Vector2(-4883.72, -11362.1),
		Vector2(-5083.06, -11561.5),
		Vector2(-5182.72, -11760.8),
		Vector2(-5481.73, -12059.8),
		Vector2(-5681.06, -12259.1),
		Vector2(-5780.73, -12259.1),
		Vector2(-5780.73, -12358.8),
		Vector2(-5880.4, -12458.5),
	};

	WarpingIsos warp1(isos);
	warp1.SetPathPoints(path1);
	warp1.PathDisplacement(1200, 0.7);
	isos = warp1.WarpedIsos();

	SetIsos(isos, false, true);

	WarpingIsos warp2(isos);
	warp2.SetPathPoints(path2);
	warp2.PathDisplacement(1200, 0.7);
	isos = warp2.WarpedIsos();

	SetIsos(isos, false, true);

	WarpingIsos warp3(isos);
	warp3.SetPathPoints(path3);
	warp3.PathDisplacement(1200, 0.7);
	isos = warp3.WarpedIsos();

	SetIsos(isos, false, true);

	WarpingIsos warp4(isos);
	warp4.SetPathPoints(path4);
	warp4.PathDisplacement(1200, 0.7);
	isos = warp4.WarpedIsos();

	SetIsos(isos, false, true);

	s = isos.ToScene(true);
	System::FlipVertical(*s, b.GetQtRect());
	System::SaveSvg(s, prefix + "isos_edit_3_river.svg", b.GetQtRect());

	ilt = IsoLineTerrain(isos);
	m_hf = ilt.SmoothStairsField(b, 1024, 1024, 1);
	m_hf.CreateImage().flipped().save(prefix + "stairs_edit_3_river.png");

	ExportPBRRender(prefix + "pbr_edit_3_river.png");
	m_hf.CreateImage().flipped().save(prefix + "mse_edit_3_river.png");

	// Création crète
	QVector<Vector2> path5 = {
		Vector2(4186.05, -11860.5),
		Vector2(4285.71, -11760.8),
		Vector2(4285.71, -11661.1),
		Vector2(5282.39, -11262.5),
		Vector2(6976.74, -11162.8),
		Vector2(8272.43, -11162.8),
		Vector2(9269.1, -11561.5),
		Vector2(10166.1, -11860.5),
		Vector2(11262.5, -11860.5),
		Vector2(12458.5, -11760.8),
		Vector2(13255.8, -11561.5),
		Vector2(13654.5, -11162.8),
		Vector2(13754.2, -11063.1),
	};
	QVector<Vector2> path6 = {
		Vector2(10066.4, -11661.1),
		Vector2(10166.1, -11661.1),
		Vector2(10166.1, -11561.5),
		Vector2(10963.5, -11461.8),
		Vector2(12059.8, -11262.5),
		Vector2(13056.5, -11162.8),
		Vector2(13455.1, -10963.5),
		Vector2(13654.5, -10465.1),
		Vector2(13654.5, -9667.77),
		Vector2(13654.5, -8970.1),
		Vector2(13654.5, -8372.09),
		Vector2(13554.8, -7973.42),
	};

	WarpingIsos warp5(isos);
	warp5.SetPathPoints(path5);
	warp5.PathDisplacement(2000, 0.75);
	isos = warp5.WarpedIsos();

	SetIsos(isos, false, true);

	WarpingIsos warp6(isos);
	warp6.SetPathPoints(path6);
	warp6.PathDisplacement(2000, 0.75);
	isos = warp6.WarpedIsos();

	SetIsos(isos, false, true);

	s = isos.ToScene(true);
	System::FlipVertical(*s, b.GetQtRect());
	System::SaveSvg(s, prefix + "isos_edit_4_crest.svg", b.GetQtRect());

	ilt = IsoLineTerrain(isos);
	m_hf = ilt.SmoothStairsField(b, 1024, 1024, 1);
	m_hf.CreateImage().flipped().save(prefix + "stairs_edit_4_crest.png");

	ExportPBRRender(prefix + "pbr_edit_4_crest.png");
	m_hf.CreateImage().flipped().save(prefix + "mse_edit_4_crest.png");

	// Création "falaise"
	QVector<Vector2> path7 = {
		Vector2(11661.1, 9867.11),
		Vector2(9966.78, 9269.1),
		Vector2(6777.41, 7674.42),
		Vector2(5681.06, 6976.74),
		Vector2(5282.39, 5980.07),
		Vector2(5681.06, 4186.05),
		Vector2(5681.06, 3588.04),
		Vector2(5282.39, 2093.02),
		Vector2(4584.72, 1196.01),
		Vector2(4485.05, 1196.01),
		Vector2(4285.71, 2591.36),
		Vector2(4684.39, 5182.72),
		Vector2(5681.06, 7873.75),
	};

	for (const Vector2 p : path7)
	{
		for (int ind = 0; ind < isos.Size(); ++ind)
		{
			isos[ind] = isos[ind].LocalDisplacement(Circle2(p, 3000), 0.1);
		}
	}

	SetIsos(isos, false, true);

	s = isos.ToScene(true);
	System::FlipVertical(*s, b.GetQtRect());
	System::SaveSvg(s, prefix + "isos_edit_5_slope.svg", b.GetQtRect());

	ilt = IsoLineTerrain(isos);
	m_hf = ilt.SmoothStairsField(b, 1024, 1024, 1);
	m_hf.CreateImage().flipped().save(prefix + "stairs_edit_5_slope.png");

	ExportPBRRender(prefix + "pbr_edit_5_slope.png");
	m_hf.CreateImage().flipped().save(prefix + "mse_edit_5_slope.png");
	*/
}

void MainAmplificationWindow::GenerateFigure28(const QString& prefix)
{
	QDir dir(prefix);
	if (!dir.exists())
		dir.mkpath(".");

	Box2 b(170000);
	int nb_isos = 12;
	int nb_particles = 10000;
	int min_height = 0;
	int max_height = 5000;
	QString sf = "../Data/dems/alps.png";

	ChangeBoxes(b);
	ChangeHistogramFromFile(sf, nb_isos);
	ChangeEstimatedNbParticles(nb_particles);
	ChangeNoise(uniform(256, 256));
	ChangeHeights(min_height, max_height);
	DefaultCamera();

	// We want to have exactly the same range for every histogram
	double histo_height_min = min_height;
	double histo_height_max = max_height;
	double histo_slope_min = 0;
	double histo_slope_max = 0.5; // arbitrary with tests
	double histo_drain_min = 0;
	double histo_drain_max = 10; // arbitrary with tests

	// We don't want exactly the same number of bins than number of isos, otherwise heights would be almost identical, we want a larger histogram.
	int nb_bins = 20;
	m_extraction_opt.histogram_palette = new AnalyticPalette(3);
	qDebug() << "Heights go from" << histo_height_min << "to" << histo_height_max;
	qDebug() << "Slopes go from" << histo_slope_min << "to" << histo_slope_max;
	qDebug() << "Drain go from" << histo_drain_min << "to" << histo_drain_max;

	// Real terrain, down sample to have the same number of pizels than particles used (100x100 pixels for 10k particles)
	HeightField real_big(b, QImage(sf).flipped(), min_height, max_height);
	HeightField real_middle(b, QImage(sf).scaled(512, 512, Qt::KeepAspectRatio, Qt::FastTransformation).flipped(), min_height, max_height);
	HeightField real_little = real_big.DownSample(36);

	HistogramD real_big_heights(real_big, nb_bins, histo_height_min, histo_height_max);
	HistogramD real_big_slopes(real_big.Slope(), nb_bins, histo_slope_min, histo_slope_max);
	HistogramD real_big_drainage(real_big.StreamArea().Ln(), nb_bins, histo_drain_min, histo_drain_max);

	ExportTerrain(real_big, prefix + "real_big.png");
	ExportTerrain(real_big.Slope(), prefix + "real_big_slopes.png");
	ExportTerrain(real_big.StreamArea().Ln(), prefix + "real_big_drain.png");
	ExportHistogram(real_big_heights, prefix + "real_big_heights_histo.svg");
	ExportHistogram(real_big_slopes, prefix + "real_big_slopes_histo.svg");
	ExportHistogram(real_big_drainage, prefix + "real_big_drain_histo.svg");

	HistogramD real_little_heights(real_little, nb_bins, histo_height_min, histo_height_max);
	HistogramD real_little_slopes(real_little.Slope(), nb_bins, histo_slope_min, histo_slope_max);
	HistogramD real_little_drainage(real_little.StreamArea().Ln(), nb_bins, histo_drain_min, histo_drain_max);

	ExportTerrain(real_little, prefix + "real_little.png");
	ExportTerrain(real_little.Slope(), prefix + "real_little_slopes.png");
	ExportTerrain(real_little.StreamArea().Ln(), prefix + "real_little_drain.png");
	ExportHistogram(real_little_heights, prefix + "real_little_heights_histo.svg");
	ExportHistogram(real_little_slopes, prefix + "real_little_slopes_histo.svg");
	ExportHistogram(real_little_drainage, prefix + "real_little_drain_histo.svg");

	// Generate terrain with different real isolines as mask
	QSet<double> heights;
	heights.insert(0);
	heights.insert(500);
	heights.insert(2000);

	IsoLines iso_mask(real_middle, heights);
	ChangeMaskFromIsos(iso_mask);

	CreateGenerationZones();
	GenerateV3();
	SetIsosFromHistogram();

	ExportMaskSVG(prefix + "gene_mask.svg");
	ExportIsosSVG(prefix + "gene_isos.svg");
	
	// interpolated
	IsoLineTerrain gene_ilt(m_isos, 0);
	HeightField gene_inter = gene_ilt.InterpolateField(b, 1024, 1024);

	HistogramD gene_inter_heights(gene_inter, nb_bins, histo_height_min, histo_height_max);
	HistogramD gene_inter_slopes(gene_inter.Slope(), nb_bins, histo_slope_min, histo_slope_max);
	HistogramD gene_inter_drainage(gene_inter.StreamArea().Ln(), nb_bins, histo_drain_min, histo_drain_max);

	ExportTerrain(gene_inter, prefix + "gene_inter.png");
	ExportTerrain(gene_inter.Slope(), prefix + "gene_inter_slopes.png");
	ExportTerrain(gene_inter.StreamArea().Ln(), prefix + "gene_inter_drain.png");
	ExportHistogram(gene_inter_heights, prefix + "gene_inter_heights_histo.svg");
	ExportHistogram(gene_inter_slopes, prefix + "gene_inter_slopes_histo.svg");
	ExportHistogram(gene_inter_drainage, prefix + "gene_inter_drain_histo.svg");

	//m_hf = HeightField(Box2(17000), gene_inter.CreateImage(), min_height, max_height);
	//UpdateGeometry();
	//ChangeCamera(250, 50, 65000);
	//RenderOpt opt = render_basic_opt(m_isos, m_hf);
	//ExportPBRRender(opt, prefix + "gene_inter_pbr.png");

	// eroded
	HeightField gene_mse = gene_ilt.InterpolateField(b, 256, 256);
	gene_mse = HeightField(Box2(17000), gene_mse.CreateImage(), min_height, max_height);
	gene_mse.ErosionAmpli();
	gene_mse = HeightField(b, gene_mse.CreateImage(), min_height, max_height);
	HistogramD gene_mse_heights(gene_mse, nb_bins, histo_height_min, histo_height_max);
	HistogramD gene_mse_slopes(gene_mse.Slope(), nb_bins, histo_slope_min, histo_slope_max);
	HistogramD gene_mse_drainage(gene_mse.StreamArea().Ln(), nb_bins, histo_drain_min, histo_drain_max);

	ExportTerrain(gene_mse, prefix + "gene_mse.png");
	ExportTerrain(gene_mse.Slope(), prefix + "gene_mse_slopes.png");
	ExportTerrain(gene_mse.StreamArea().Ln(), prefix + "gene_mse_drain.png");
	ExportHistogram(gene_mse_heights, prefix + "gene_mse_heights_histo.svg");
	ExportHistogram(gene_mse_slopes, prefix + "gene_mse_slopes_histo.svg");
	ExportHistogram(gene_mse_drainage, prefix + "gene_mse_drain_histo.svg");

	//m_hf = HeightField(Box2(17000), gene_mse.CreateImage(), min_height, max_height);
	//UpdateGeometry();
	//ChangeCamera(250, 50, 65000);
	//opt = render_basic_opt(m_isos, m_hf);
	//ExportPBRRender(opt, prefix + "gene_mse_pbr.png");

	// Final render of the real terrain 
	//m_hf = HeightField(Box2(17000), real.CreateImage(), min_height, max_height);
	//ChangeCamera(250, 50, 65000);
	//UpdateGeometry();
	//opt = render_basic_opt(m_isos, m_hf);
	//ExportPBRRender(opt, prefix + "real_pbr.png");
}

void MainAmplificationWindow::GenerateFigurePresentation1(const QString& prefix)
{
	QDir dir(prefix);
	if (!dir.exists())
		dir.mkpath(".");

	int crop_x = 10;
	int crop_y = 10;
	Box2 b(500);

	QImage imghf = QImage("../Data/dems/rockies_zoom_breach.png").flipped();
	ScalarField2 sf = HeightField(b, imghf, 0, 2500);
	int x = imghf.width();
	int y = imghf.height();
	sf.Scale(100);
	b.Scale(100);

	QImage img = sf.CreateImage(AnalyticPalette(3)).flipped();
	img.save(prefix + "grid.png");
	QRect cropArea(0, y - crop_y, crop_x, crop_y);  // x, y, width, height
	QImage subset = img.copy(cropArea);
	subset.save(prefix + "grid_zoom.png");

	Vector2 base = b.Vertex(2);
	Vector2 move = b.Vertex(2) + Vector2((100000 * crop_x) / x, -(100000 * crop_y) / y);
	Box2 zoom(Vector2::Min(base, move), Vector2::Max(base, move));

	QImage isoimg = QImage("../Data/dems/rockies_zoom_breach.png").flipped().scaled(256, 256, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	ScalarField2 isosf = HeightField(b, isoimg, 0, 2500);
	IsoLines isos(isosf, 20);

	IsoLines::DisplayOptions opt;
	opt.fill = true;

	QGraphicsScene* scene = isos.ToScene(opt);
	System::FlipVertical(*scene, b.GetQtRect());
	System::SaveSvg(scene, prefix + "isos.svg", b.GetQtRect());

	scene = isos.ToScene(opt);
	System::FlipVertical(*scene, zoom.GetQtRect());
	System::SaveSvg(scene, prefix + "isos_zoom.svg", zoom.GetQtRect());
}

void MainAmplificationWindow::GenerateFigurePresentation2(const QString& prefix)
{
	QDir dir(prefix + "gifs/");
	if (!dir.exists())
		dir.mkpath(".");

	int nb_isos = 12;
	int nb_particles = 150;

	IsoLines isos({ PolygonReunion().Scaled(0.01) });
	Box2 b = isos.GetBox().ScaledCentered(1.2);

	ChangeBoxes(b);
	ChangeMaskFromIsos(isos);
	ChangeEstimatedNbParticles(nb_particles);
	ChangeNoise(uniform(256, 256));

	ExportMaskSVG(prefix + "mask.svg");

	Generate(1);

	m_extraction_opt.graph_white_background = false;
	m_extraction_opt.histogram_palette = new AnalyticPalette(3);

	// How to generate
	//ExportGraphResultGif(prefix + "gifs/");
	ExportGraphResult(prefix + "graph.svg");

	// Different Histo
	ChangeHistogramFromFunction(3, nb_isos);
	ExportHistogram(prefix + "histo_1.svg");
	ExportGraphHeights(prefix + "graph_h1.svg");
	ExportGraphsEachHeight(prefix + "graph-");

	ChangeHistogramFromFunction(0, nb_isos);
	ExportHistogram(prefix + "histo_2.svg");
	ExportGraphHeights(prefix + "graph_h2.svg");

	ChangeHistogramFromFunction(4, nb_isos);
	ExportHistogram(prefix + "histo_3.svg");
	ExportGraphHeights(prefix + "graph_h3.svg");

	ChangeHistogramFromFile("../Data/dems/reunion.png", nb_isos);
	ExportHistogram(prefix + "histo_4.svg");
	ExportGraphHeights(prefix + "graph_h4.svg");
	return;

	// Isolines Extraction
	for (int i = 0; i < m_generation_histogram.Size(); ++i)
	{
		m_extraction_opt = ExtractionOpt();
		m_extraction_opt.isos_opt.max_isos = i;
		m_extraction_opt.isos_white_background = false;
		ExportIsosSVG(prefix + "isos" + QString::number(i+1) + ".svg");

		m_extraction_opt.isos_opt.fill = false;
		m_extraction_opt.isos_opt.min_isos = i;
		m_extraction_opt.isos_opt.edge_size = 3;
		m_extraction_opt.isos_opt.palette = new Palette(QVector<Color>({ Color(161, 70, 126) }));
		ExportIsosSVG(prefix + "isos_single" + QString::number(i + 1) + ".svg");
	}

	m_extraction_opt = ExtractionOpt();
	m_extraction_opt.graph_white_background = false;
	m_extraction_opt.isos_white_background = false;

	// Different N + isolines
	for (int nb_particles : {150, 500, 2000, 10000})
	{
		ChangeEstimatedNbParticles(500);
		Generate(1);
		ExportGraphResult(prefix + "graph_" + QString::number(nb_particles) + "_eden.svg");
		ExportGraphHeights(prefix + "graph_" + QString::number(nb_particles) + ".svg");
		ExportIsosSVG(prefix + "isos_" + QString::number(nb_particles) + ".svg");
	}

	m_extraction_opt = ExtractionOpt();
}

void MainAmplificationWindow::GenerateFigurePresentation3(const QString& prefix)
{
	QDir dir(prefix);
	if (!dir.exists())
		dir.mkpath(".");

	// Réunion
	int nb_isos = 12;
	IsoLines isos({ PolygonReunion().Scaled(0.01) });
	Box2 b = isos.GetBox().ScaledCentered(1.2);

	m_extraction_opt.isos_white_background = false;

	ChangeBoxes(b);
	ChangeMaskFromIsos(isos);
	ChangeHistogramFromFile("../Data/dems/reunion.png", nb_isos);
	ExportMaskSVG(prefix + "reunion_mask.svg");

	// Noises
	QMap<QString, ScalarField2> noises = {
		{"uniform_1", uniform(256, 256)},
		{"uniform_2", uniform(256, 256)},
		{"fbm", fbm(256, 256, 10)}
	};
	ChangeNoise(noises["uniform_1"]);
	ExportNoise(prefix + "reunion_uniform.png");
	ChangeNoise(noises["fbm"]);
	ExportNoise(prefix + "reunion_fbm.png");

	for (double nb_particles : {150, 500, 2000, 10000})
	{
		ChangeEstimatedNbParticles(nb_particles);
		CreateGenerationZones();

		for (QString noise_name : noises.keys())
		{
			ScalarField2 noise = noises[noise_name];
			ChangeNoise(noise);
			GenerateV1();
			SetIsosFromHistogram();

			ExportIsosSVG(prefix + "reunion_" + noise_name + "_isos_" + QString::number(nb_particles) + ".svg");
		}
	}

	// Hawai
	nb_isos = 12;
	isos = IsoLines({ PolygonHawaii().Scaled(0.013) });
	b = isos.GetBox().ScaledCentered(1.2);

	ChangeBoxes(b);
	ChangeMaskFromIsos(isos);
	ChangeHistogramFromFile("../Data/dems/reunion.png", nb_isos);
	ExportMaskSVG(prefix + "hawaii_mask.svg");

	// Noises
	ChangeNoise(uniform(256, 256));
	ExportNoise(prefix + "hawaii_uniform.png");
	ChangeNoiseFromFile("../Images/input/hawaii_rivers.png");
	ExportNoise(prefix + "hawaii_rivers.png");

	for (double nb_particles : {150, 500, 2000, 10000})
	{
		ChangeEstimatedNbParticles(nb_particles);
		CreateGenerationZones();

		ChangeNoise(uniform(256, 256));
		GenerateV1();
		SetIsosFromHistogram();

		ExportIsosSVG(prefix + "hawaii_uniform_isos_" + QString::number(nb_particles) + ".svg");

		ChangeNoiseFromFile("../Images/input/hawaii_rivers.png");
		GenerateV1();
		SetIsosFromHistogram();

		ExportIsosSVG(prefix + "hawaii_rivers_isos_" + QString::number(nb_particles) + ".svg");
	}

	m_extraction_opt = ExtractionOpt();
}

void MainAmplificationWindow::GenerateFigurePresentation4(const QString& prefix)
{
	QDir dir1(prefix + "gifs-bad/");
	if (!dir1.exists())
		dir1.mkpath(".");
	QDir dir2(prefix + "gifs-good/");
	if (!dir2.exists())
		dir2.mkpath(".");

	int nb_isos = 12;
	int nb_particles = 250;
	Box2 b(500);
	
	m_extraction_opt.graph_white_background = false;

	ChangeBoxes(b);
	// Masque 3 zones
	ChangeEstimatedNbParticles(nb_particles);
	ChangeMaskFromFile("../images/input/mask_boundaries.png", 3);
	ChangeHistogramFromFile("../Data/dems/rockies_zoom_breach.png", nb_isos);
	ChangeNoise(uniform(256, 256));

	CreateGenerationZones();
	GenerateV2();

	ExportMaskSVG(prefix + "mask.svg");
	ChangeBoxes(m_generation_box.Scaled(1.1)); // To avoid cutting nodes of the graph
	ExportGraphZonesSplit(prefix + "graph-");
	ExportGraphResultSplit(prefix + "graph-bad-");
	ExportGraphResultGif(prefix + "gifs-bad/");

	ChangeBoxes(b);

	GenerateV3();

	ChangeBoxes(m_generation_box.Scaled(1.1)); // To avoid cutting nodes of the graph
	ExportGraphsDoubleEdenGif(prefix + "gifs-good/");
	ExportGraphResultSplit(prefix + "graph-good-");


}

void MainAmplificationWindow::GenerateFigurePresentation5(const QString& prefix)
{
	QDir dir(prefix);
	if (!dir.exists())
		dir.mkpath(".");

	int nb_isos = 12;
	int nb_particles = 12000;
	double min_height = 0;
	double max_height = 150;
	Box2 b(500);

	ChangeBoxes(b);
	ChangeEstimatedNbParticles(nb_particles);
	ChangeMaskFromFile("../images/input/teaser-old-2.png", 1);
	ChangeHistogramFromFile("../Data/dems/rockies_zoom_breach.png", nb_isos);
	ChangeNoise(fbm(256, 256, 10));
	ChangeHeights(min_height, max_height);

	Generate(1);

	m_extraction_opt.terrain_palette = new AnalyticPalette(3);

	ExportIsosSVG(prefix + "isos.svg");
	ExportIsosStairs(prefix + "stairs.png", 1024, 1024);
	ExportIsosInterpolate(prefix + "interpolate.png", 1024, 1024);

	RenderOpt opt = render_basic_opt(m_isos, m_hf);
	opt.main_isolines_width = 0;
	opt.lesser_isolines_width = 0;
	opt.sky_color = Color(237, 237, 243);

	IsoLineTerrain ilt(m_isos);
	m_hf = ilt.InterpolateField(m_edition_box, 1024, 1024);

	UpdateGeometry();
	ChangeCamera(250, 30, 2500);
	ExportPBRRender(opt, prefix + "interpolate_pbr.png");

	ilt = IsoLineTerrain(m_isos);
	m_hf = ilt.SmoothStairsField(m_edition_box, 1024, 1024);

	UpdateGeometry();
	ExportPBRRender(opt, prefix + "stairs_pbr.png");

	delete m_extraction_opt.terrain_palette;
	m_extraction_opt = ExtractionOpt();
}

void MainAmplificationWindow::GenerateFigurePresentation6(const QString& prefix)
{
	QDir dir(prefix);
	if (!dir.exists())
		dir.mkpath(".");

	int nb_particles = 10000;
	double min_height = 0;
	double max_height = 3500;
	int nb_isos = 12;

	RenderOpt opt;
	opt.nb_dispatch = 4;
	opt.anti_aliasing = 2;
	opt.sea_level = 0.05;
	opt.terrain_color = Color::Grey(0.9);
	opt.sea_deep_color = Color(209, 234, 255);
	opt.sea_shore_color = opt.sea_deep_color;

	m_extraction_opt.histogram_palette = new AnalyticPalette(3);

	ChangeEstimatedNbParticles(nb_particles);
	ChangeHeights(min_height, max_height);

	// First example
	IsoLines isos = IsoLines({ PolygonHawaii().Scaled(0.35)}).Centered();
	Box2 b = isos.GetBox().ScaledCentered(1.2);

	ChangeBoxes(b);
	ChangeMaskFromIsos(isos);
	ChangeNoise(fbm(256, 256, 10));
	ChangeHistogramFromFile("../Data/dems/reunion.png", nb_isos);

	Generate(3);

	ExportMaskSVG(prefix + "1_mask.svg");
	ExportNoise(prefix + "1_noise.png");
	ExportHistogram(prefix + "1_histo.svg");
	ExportIsosSVG(prefix + "1_isos.svg");

	ChangeCamera(200, 30, 50000, Vector(0, 0, -1500));

	IsoLineTerrain ilt(m_isos);
	m_hf = ilt.InterpolateField(m_edition_box, 256, 256);
	MSE(4);
	ExportPBRRender(opt, prefix + "1_mse_pbr.png");

	m_hf = ilt.SmoothStairsField(m_edition_box, 2048, 2048);

	UpdateGeometry();
	ExportPBRRender(opt, prefix + "1_stairs_pbr.png");

	// Second example
	isos = IsoLines({ PolygonReunion().Scaled(0.3)}).Centered();
	b = isos.GetBox().ScaledCentered(1.2);

	ChangeBoxes(b);
	ChangeMaskFromIsos(isos);
	ChangeNoiseFromFile("../Images/input/reunion_rivers.png", 2);
	ChangeNoise(m_generation_noise + .1 * fbm(m_generation_noise.GetSizeX(), m_generation_noise.GetSizeY(), 10));
	ChangeHistogramFromFile("../Data/dems/reunion.png", nb_isos);

	Generate(3);

	ExportMaskSVG(prefix + "2_mask.svg");
	ExportNoise(prefix + "2_noise.png");
	ExportHistogram(prefix + "2_histo.svg");
	ExportIsosSVG(prefix + "2_isos.svg");

	ChangeCamera(200, 30, 50000, Vector(0, 0, -1500));

	ilt = IsoLineTerrain(m_isos);
	m_hf = ilt.InterpolateField(m_edition_box, 256, 256);
	MSE(4, 1, 0.8);
	ExportPBRRender(opt, prefix + "2_mse_pbr.png");

	m_hf = ilt.SmoothStairsField(m_edition_box, 2048, 2048);

	UpdateGeometry();
	ExportPBRRender(opt, prefix + "2_stairs_pbr.png");

	// Third example
	b = Box2(15000);

	ChangeBoxes(b);
	ChangeMaskFromFile("../images/input/multi_mask_2.png", 3);
	ChangeNoise(uniform(256, 256));
	ChangeHistogramFromFile("../Data/dems/rockies_zoom_breach.png", nb_isos);

	Generate(3);

	ExportMaskSVG(prefix + "3_mask.svg");
	ExportNoise(prefix + "3_noise.png");
	ExportHistogram(prefix + "3_histo.svg");
	ExportIsosSVG(prefix + "3_isos.svg");

	ChangeCamera(20, 30, 50000, Vector(0, 0, -1500));

	ilt = IsoLineTerrain(m_isos);
	m_hf = ilt.InterpolateField(m_edition_box, 256, 256);
	MSE(4, 1, 0.6);
	ExportPBRRender(opt, prefix + "3_mse_pbr.png");

	m_hf = ilt.SmoothStairsField(m_edition_box, 2048, 2048);

	UpdateGeometry();
	ExportPBRRender(opt, prefix + "3_stairs_pbr.png");

	delete m_extraction_opt.histogram_palette;
	m_extraction_opt = ExtractionOpt();
}

///////////////////////////////
////      PARAMETERS       ////
///////////////////////////////

void MainAmplificationWindow::ResetHistogramView()
{
	if (generate_figures)
		return;

	QGraphicsScene* scene = new QGraphicsScene;
	m_generation_histogram.Draw(*scene, AnalyticPalette(3), 100, 50);
	m_uiw.histogram_view->setScene(scene);
	m_uiw.histogram_view->resetTransform();
	m_uiw.histogram_view->fitInView(scene->sceneRect());
	m_uiw.histogram_view->scale(1, -1);
}

void MainAmplificationWindow::ChangeHistogram(const HistogramD& histo)
{
	m_generation_histogram = histo;
	m_uiw.histogram_nb_isos_edit->setText(QString::number(histo.Size()));
	
	ResetHistogramView();
	SetIsosFromHistogram();
}

void MainAmplificationWindow::ChangeHistogramFromFile(const QString& file, int size)
{
	size = Math::Max(1, size);
	m_generation_histogram_file = file;
	m_uiw.histogram_function_selector->setCurrentIndex(0);
	// Pour que l'image ne soit pas trop grosse
	ChangeHistogram(histo_from_hf(ScalarField2(QImage(file).scaled(512, 512, Qt::KeepAspectRatio, Qt::FastTransformation)), size));
}

void MainAmplificationWindow::ChangeHistogramFromFunction(int i, int size)
{
	size = Math::Max(1, size);
	m_uiw.histogram_function_selector->setCurrentIndex(i + 1);
	ChangeHistogram(common_histo_from_function(i, size));
}

// convenient
void MainAmplificationWindow::ChangeHistogramFromUI()
{
	int id = m_uiw.histogram_function_selector->currentIndex();
	int size = m_uiw.histogram_nb_isos_edit->text().toInt();
	if (id == 0)
		ChangeHistogramFromFile(m_generation_histogram_file, size);
	else
		ChangeHistogramFromFunction(id - 1, size);
}

void MainAmplificationWindow::BrowseHistogram()
{
	QString fileName = QFileDialog::getOpenFileName(
		this,
		tr("Choose File"),
		m_histogram_directory,
		tr("Images (*.png *.jpg *.jpeg *.bmp *.gif)")
	);

	if (!fileName.isEmpty())
	{
		ChangeHistogramFromFile(fileName, m_uiw.histogram_nb_isos_edit->text().toInt());
	}
}

void MainAmplificationWindow::ResetNoiseView()
{
	if (generate_figures)
		return;

	IsoLines isos = isolines_mask(m_generation_mask);
	QImage image = proba_inside_isos(m_generation_noise, isos);
	QPixmap pix = QPixmap::fromImage(image);
	qreal scaleX = m_generation_box.Width() / pix.width();
	qreal scaleY = m_generation_box.Height() / pix.height();

	QGraphicsScene* scene = new QGraphicsScene;
	QGraphicsPixmapItem* item = scene->addPixmap(pix);
	item->setTransform(QTransform::fromScale(scaleX, scaleY));
	m_uiw.noise_view->setScene(scene);
	m_uiw.noise_view->resetTransform();
	m_uiw.noise_view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
	m_uiw.noise_view->scale(1, -1);
}

void MainAmplificationWindow::ChangeNoise(const ScalarField2& noise, double pow)
{
	m_generation_noise_without_pow = noise;
	m_generation_noise = change_sf_box(m_generation_noise_without_pow, m_generation_box);
	if (pow != 1)
		m_generation_noise.Pow(pow);
	m_uiw.noise_pow_edit->setText(QString::number(pow));
	ResetNoiseView();
}

void MainAmplificationWindow::ChangeNoiseFromFile(const QString& file, double pow)
{
	m_generation_noise_file = file;
	// Pour que l'image ne soit pas trop grosse
	ChangeNoise(ScalarField2(QImage(file).scaled(512, 512, Qt::KeepAspectRatio, Qt::FastTransformation).flipped()), pow);
}

void MainAmplificationWindow::ChangeNoisePower(double pow)
{
	ChangeNoise(m_generation_noise_without_pow, pow);
}

void MainAmplificationWindow::BrowseNoise()
{
	QString fileName = QFileDialog::getOpenFileName(
		this,
		tr("Choose File"),
		m_noise_directory,
		tr("Images (*.png *.jpg *.jpeg *.bmp *.gif)")
	);

	if (!fileName.isEmpty())
	{
		double pow = m_uiw.noise_pow_edit->text().toDouble();
		ChangeNoiseFromFile(fileName, pow);
	}
}

void MainAmplificationWindow::ResetMaskView()
{
	if (generate_figures)
		return;

	QGraphicsScene* scene = isolines_mask_scene(m_generation_mask, m_extraction_opt.mask_palette, true, true);
	m_uiw.mask_view->setScene(scene);
	m_uiw.mask_view->resetTransform();
	m_uiw.mask_view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
	m_uiw.mask_view->scale(1, -1);
}

void MainAmplificationWindow::ChangeMask(const ScalarField2& sf)
{
	m_generation_mask = change_sf_box(sf, m_generation_box);
	m_generation_mask.Normalize();
	ResetMaskView();

	// noise view depends on the mask view
	if (m_generation_noise.VertexSize() != 0)
		ResetNoiseView();
	ResetPoissonRadius();
}

void MainAmplificationWindow::ChangeMaskFromFile(const QString& file, int nb_isos, double sea_level, int smooth_number)
{
	nb_isos = Math::Max(1, nb_isos);
	sea_level = Math::Max(0.01, Math::Min(0.99, sea_level));

	m_generation_mask_file = file;
	m_uiw.mask_nb_isos_edit->setText(QString::number(nb_isos));
	m_uiw.sea_level_edit->setText(QString::number(sea_level));

	// Pour que l'image ne soit pas trop grosse
	ScalarField2 mask_field(m_generation_box, QImage(file).scaled(512, 512, Qt::KeepAspectRatio, Qt::FastTransformation).flipped(), 0, 1);
	ChangeMask(thresholding(mask_field, nb_isos, sea_level, smooth_number));
}

void MainAmplificationWindow::ChangeMaskFromMaskFile(const QString& file)
{
	ScalarField2 sf(QImage(file).flipped());

	// Pour mettre un meme pas entre toutes les zones
	QSet<double> vals;
	for (double v : sf)
		vals.insert(v);
	int size = vals.size();
	QVector<double> sorted(vals.begin(), vals.end());
	sort(sorted.begin(), sorted.end());
	for (double& v : sf)
		for (int i = 0; i < size; ++i)
			if (sorted[i] == v)
				v = i / (size - 1.);

	qDebug() << "mask size: " << size;
	ChangeMask(sf);
}

void MainAmplificationWindow::ChangeMaskParams(int nb_isos, double sea_level)
{
	ChangeMaskFromFile(m_generation_mask_file, nb_isos, sea_level);
}

void MainAmplificationWindow::ChangeMaskFromIsos(const IsoLines& isos)
{
	if (isos.Size() == 0)
		return;

	// Si toutes les isos sont à la même hauteur, c'est que tout a été fait à la main, donc on adapte les hauteurs
	// TODO: c'est pas très beau mais pas le temps
	IsoLines isosCopy = isos;
	if (isos.Heights().size() == 1)
	{
		isosCopy.ChangeHeightsMinMaxEndoreic(m_min_height, m_max_height);
	}

	ChangeMask(mask_from_isos(m_generation_box, isosCopy, 512, 512));
}

void MainAmplificationWindow::ChangeMaskFromCurrentIsos()
{
	ChangeMaskFromIsos(m_isos);
}

void MainAmplificationWindow::BrowseMask()
{
	QString fileName = QFileDialog::getOpenFileName(
		this,
		tr("Choose File"),
		m_mask_directory,
		tr("Images (*.png *.jpg *.jpeg *.bmp *.gif)")
	);

	if (!fileName.isEmpty())
	{
		ChangeMaskFromFile(fileName, m_uiw.mask_nb_isos_edit->text().toInt(), m_uiw.sea_level_edit->text().toDouble());
	}
}

void MainAmplificationWindow::ResetPoissonRadius()
{
	// Il est plus simple de permettre à l'utilisateur de choisir le nombre de particules N
	// Cependant, la génération demande non pas un nombre N mais un rayon r, donc on estime le nombre r en fonction des paramètres
	// Si les particules sont toutes collées dans un carré, pour un rayon r, une auteur h et une largeur w, on a N = w/r * h/r = (w*h)/r^2
	// Pour un contour C quelconque on peut considérer que h*r = aire(C), de plus, comme on n'a évidemment pas la perfection alignées, on divise le tout par un ratio à définir :
	// N = aire(C) / (r * ratio)^2
	// De là on obtient
	// r = sqrt(aire(C) / N) / ratio
	// Après différents tests, il apparait que ratio = 2.62 est une bonne valeur, mais cela dépend évidemment du contour.
	IsoLines isos = isolines_mask(m_generation_mask);
	double area = 0;
	for (int i : isos.Roots())
		area += isos.At(i).Area();
	double ratio = 2.62;
	int n = Math::Max(1, m_uiw.poisson_radius_edit->text().toInt());

	m_uiw.poisson_radius_edit->setText(QString::number(n));
	m_generation_radius = sqrt(area / n) / ratio;
}

void MainAmplificationWindow::ChangeEstimatedNbParticles(int n)
{
	m_uiw.poisson_radius_edit->setText(QString::number(n));
	ResetPoissonRadius();
}

void MainAmplificationWindow::ResetGenBox()
{
	double center_x = m_uiw.generation_box_x_edit->text().toDouble() * 1000;
	double center_y = m_uiw.generation_box_y_edit->text().toDouble() * 1000;
	double width = m_uiw.generation_box_w_edit->text().toDouble() * 1000;
	double height = m_uiw.generation_box_h_edit->text().toDouble() * 1000;
	Vector2 center_box(center_x, center_y);
	m_generation_box = Box2(center_box, width, height);

	// évite les bugs si pas encore de mask
	if (m_generation_mask.VertexSize() == 0)
		return;

	ChangeMask(m_generation_mask);
	ChangeNoise(m_generation_noise);
}

void MainAmplificationWindow::ChangeGenBox(const Box2& b)
{
	m_uiw.generation_box_x_edit->setText(QString::number(b.Center()[0] / 1000.));
	m_uiw.generation_box_y_edit->setText(QString::number(b.Center()[1] / 1000.));
	m_uiw.generation_box_w_edit->setText(QString::number(b.Width() / 1000.));
	m_uiw.generation_box_h_edit->setText(QString::number(b.Height() / 1000.));
	ResetGenBox();
}

void MainAmplificationWindow::ResetViewBox()
{
	double center_x = m_uiw.view_box_x_edit->text().toDouble() * 1000;
	double center_y = m_uiw.view_box_y_edit->text().toDouble() * 1000;
	double width = m_uiw.view_box_w_edit->text().toDouble() * 1000;
	double height = m_uiw.view_box_h_edit->text().toDouble() * 1000;
	Vector2 center_box(center_x, center_y);
	m_edition_box = Box2(center_box, width, height);

	m_uiw.iso_view->SetViewRect(m_edition_box);
	m_uiw.iso_view->ResetView();
	ResetCamera();
}

void MainAmplificationWindow::ChangeViewBox(const Box2& b)
{
	m_uiw.view_box_x_edit->setText(QString::number(b.Center()[0] / 1000.));
	m_uiw.view_box_y_edit->setText(QString::number(b.Center()[1] / 1000.));
	m_uiw.view_box_w_edit->setText(QString::number(b.Width() / 1000.));
	m_uiw.view_box_h_edit->setText(QString::number(b.Height() / 1000.));
	ResetViewBox();
}

void MainAmplificationWindow::ChangeBoxes(const Box2& b)
{
	ChangeViewBox(b);
	ChangeGenBox(b);
}

void MainAmplificationWindow::ResetHeights()
{
	m_min_height = m_uiw.heights_min_edit->text().toDouble();
	m_max_height = m_uiw.heights_max_edit->text().toDouble();
	m_uiw.heights_min_edit->setText(QString::number(m_min_height));
	m_uiw.heights_max_edit->setText(QString::number(m_max_height));

	// On ne change pas les isos, mais on veut modifier la hauteurs de celles affichées
	SetIsos(m_isos);
}

void MainAmplificationWindow::ChangeHeights(double min, double max)
{
	m_uiw.heights_min_edit->setText(QString::number(min));
	m_uiw.heights_max_edit->setText(QString::number(max));
	ResetHeights();
}

void MainAmplificationWindow::ResetTerrainSize()
{
	m_terrain_x = m_uiw.terrain_x_edit->text().toDouble();
	m_terrain_y = m_uiw.terrain_y_edit->text().toDouble();
	m_uiw.terrain_x_edit->setText(QString::number(m_terrain_x));
	m_uiw.terrain_y_edit->setText(QString::number(m_terrain_y));

	m_terrain_type = (TerrainType) m_uiw.terrain_type_group->checkedId();

	RecomputeTerrainFromIsos();
}

void MainAmplificationWindow::ChangeTerrainSize(double x, double y)
{
	m_uiw.terrain_x_edit->setText(QString::number(x));
	m_uiw.terrain_y_edit->setText(QString::number(y));
	ResetTerrainSize();
}

void MainAmplificationWindow::ResetCamera()
{
	m_camera_longitude = Math::Max(0, Math::Min(360, m_uiw.camera_longitude_angle_edit->text().toDouble()));
	m_camera_latitude = Math::Max(-90, Math::Min(90, m_uiw.camera_latitude_angle_edit->text().toDouble()));
	m_camera_dist = m_uiw.camera_dist_edit->text().toDouble();
	double lookat_x = m_uiw.camera_lookat_x_edit->text().toDouble();
	double lookat_y = m_uiw.camera_lookat_y_edit->text().toDouble();
	double lookat_z = m_uiw.camera_lookat_z_edit->text().toDouble();
	m_camera_lookat = Vector(lookat_x, lookat_y, lookat_z);

	if (m_camera_dist <= 0)
		m_camera_dist = Norm(m_edition_box.Diagonal()) * 1.5;

	m_uiw.camera_longitude_angle_edit->setText(QString::number(m_camera_longitude));
	m_uiw.camera_latitude_angle_edit->setText(QString::number(m_camera_latitude));
	m_uiw.camera_dist_edit->setText(QString::number(m_camera_dist));

	// Adapt camera far plane to the terrain.
	Box bbox(Vector(m_edition_box[0][0], m_edition_box[0][1], m_min_height), Vector(m_edition_box[1][0], m_edition_box[1][1], m_max_height));
	double size = bbox.Size().Max();
	double cameraNearPlane = 1.0;
	double cameraFarPlane = size * 4.0;

	// Set view point from parameter
	Box2 box = m_edition_box;
	double r = m_camera_dist;
	double longitude_rad = m_camera_longitude * M_PI / 180;
	double latitude_rad = m_camera_latitude * M_PI / 180;
	Vector eye = m_camera_lookat + r * Vector(
		cos(latitude_rad) * cos(longitude_rad),
		cos(latitude_rad) * sin(longitude_rad),
		sin(latitude_rad));

	// Update MayaWidget camera
	Camera cam;
	cam.SetEye(eye);
	cam.SetAt(m_camera_lookat);

	m_meshWidget->SetNearAndFarPlane(Vector2(cameraNearPlane, cameraFarPlane));
	m_meshWidget->SetCamera(cam);
}

/*
 * Set the view of the camera on the circle around the center of the current terrain
 *
 * \param theta : [0, 360]
 * \param phi : [0, 180]
 * \param dist : The distance factor to the terrain
 */
void MainAmplificationWindow::ChangeCamera(double longitude, double latitude, double dist, const Vector& lookat)
{
	m_uiw.camera_longitude_angle_edit->setText(QString::number(longitude));
	m_uiw.camera_latitude_angle_edit->setText(QString::number(latitude));
	m_uiw.camera_dist_edit->setText(QString::number(dist));
	m_uiw.camera_lookat_x_edit->setText(QString::number(lookat[0]));
	m_uiw.camera_lookat_y_edit->setText(QString::number(lookat[1]));
	m_uiw.camera_lookat_z_edit->setText(QString::number(lookat[2]));
	ResetCamera();
}

void MainAmplificationWindow::ChangeCamera(const Camera& c)
{
	//m_meshWidget->SetCamera(c);

	// On récupère les coordonnées sphériques et on les affiches dans l'ui
	Vector v = c.Eye();
	Vector d = c.View(); // direction
	double r = Norm(d);
	Vector u = d / r;
	// Normalement je devrais avoir lat = sin(z) et long = atan2(y, x)
	// Mais je sais pas pourquoi je dois faire lat = -sin(x) et long = atan2(y, x) + pi
	double latitude_rad = -asin(u[2]);
	double longitude_rad = atan2(u[1], u[0]) + M_PI;

	m_camera_longitude = longitude_rad * 180 / M_PI;
	m_camera_latitude = latitude_rad * 180 / M_PI;
	m_camera_dist = r;
	m_camera_lookat = c.At();

	m_uiw.camera_longitude_angle_edit->setText(QString::number(m_camera_longitude));
	m_uiw.camera_latitude_angle_edit->setText(QString::number(m_camera_latitude));
	m_uiw.camera_dist_edit->setText(QString::number(m_camera_dist));
	m_uiw.camera_lookat_x_edit->setText(QString::number(m_camera_lookat[0]));
	m_uiw.camera_lookat_y_edit->setText(QString::number(m_camera_lookat[1]));
	m_uiw.camera_lookat_z_edit->setText(QString::number(m_camera_lookat[2]));
}

void MainAmplificationWindow::DefaultCamera()
{
	//ChangeCamera(225, 26.565);
	ChangeCamera(250, 50);
}

void MainAmplificationWindow::LookDownCamera()
{
	ChangeCamera(270, 90);
}

void MainAmplificationWindow::ResetEditionOptions()
{
	m_edition_radius = Math::Max(0.1, m_uiw.edition_radius_edit->text().toDouble());
	m_edition_point_min_dist = Math::Max(0.1, m_uiw.point_min_dist_edit->text().toDouble());
	m_uiw.edition_radius_edit->setText(QString::number(m_edition_radius));
	m_uiw.point_min_dist_edit->setText(QString::number(m_edition_point_min_dist));

	m_uiw.iso_view->SetRadius(m_edition_radius);
	m_uiw.expected_points_text->setText("> " + QString::number(Math::Ceil(m_isos.TotalLength() / m_edition_point_min_dist)) + " pts");
}

void MainAmplificationWindow::ResetSmoothingOptions()
{
	m_smoothing_delta = Math::Max(0, Math::Min(1, m_uiw.edition_delta_edit->value()));
	m_smoothing_dist = m_uiw.edition_dist_edit->text().toDouble();
	m_uiw.edition_delta_edit->setValue(m_smoothing_delta);
	m_uiw.edition_dist_edit->setText(QString::number(m_smoothing_dist));
}

void MainAmplificationWindow::ResetWarpingOptions()
{
	m_warping_pow = m_uiw.warp_pow_edit->value();
	m_warping_strength = m_uiw.warp_strength_edit->value();
	m_warping_factor = m_uiw.warp_factor_eps_edit->value();
}

void MainAmplificationWindow::ResetSlopingOptions()
{
	m_sloping_pow = m_uiw.slope_pow_edit->value();
	m_sloping_strength = m_uiw.slope_strength_edit->value();
}

void MainAmplificationWindow::ResetProtectingOptions()
{
	m_protecting_remove = m_uiw.toggle_remove_circles->isChecked();
	m_protecting_iso_max_length = Math::Max(0, m_uiw.protect_small_isolines_edit->text().toDouble());
	m_uiw.protect_small_isolines_edit->setText(QString::number(m_protecting_iso_max_length));

	bool show = m_uiw.toggle_show_zones->isChecked();
	m_uiw.iso_view->DisplayProtectedZones(show);
}

void MainAmplificationWindow::ChangeEditionTool(int index)
{
	m_current_tool = (EditTool)index;
	switch (m_current_tool)
	{
	case DRAWING:
		m_uiw.iso_view->DisplayPath(true);
		m_uiw.iso_view->DisplayCircle(false);
		break;
	case SMOOTHING:
	case WARPING:
	case SLOPING:
	case PROTECTING:
		m_uiw.iso_view->DisplayPath(true);
		m_uiw.iso_view->DisplayCircle(true);
		break;
	default:
		qWarning() << "Tool not exists";
		break;
	}

	m_uiw.options_pages->setCurrentIndex(index);
	m_uiw.edition_tool_selector->setCurrentIndex(index);
}

void MainAmplificationWindow::ChangeEditionRadius(double radius)
{
	m_uiw.edition_radius_edit->setText(QString::number(radius));
	ResetEditionOptions();
}

void MainAmplificationWindow::ChangeEditionResamplingMinDist(double min_dist)
{
	m_uiw.point_min_dist_edit->setText(QString::number(min_dist));
	ResetEditionOptions();
}

void MainAmplificationWindow::ChangeSmoothingDistOption(double dist)
{
	m_uiw.edition_dist_edit->setText(QString::number(dist));
	ResetSmoothingOptions();
}

void MainAmplificationWindow::ChangeSmoothingDeltaOption(double delta)
{
	m_uiw.edition_delta_edit->setValue(delta);
	ResetSmoothingOptions();
}

void MainAmplificationWindow::ChangeProtectionMaxLength(double max_length)
{
	m_uiw.protect_small_isolines_edit->setText(QString::number(max_length));
	ResetProtectingOptions();
}

void MainAmplificationWindow::ResetExportOptions()
{
	m_extraction_opt.extract_isos = m_uiw.export_isos_toggle->isChecked();
	m_extraction_opt.extract_pbr = m_uiw.export_pbr_toggle->isChecked();
	m_extraction_opt.extract_stairs = m_uiw.export_stairs_toggle->isChecked();
	m_extraction_opt.extract_smoothed_stairs = m_uiw.export_smooth_toggle->isChecked();
	m_extraction_opt.extract_interpolate = m_uiw.export_inter_toggle->isChecked();
}

void MainAmplificationWindow::ChangeExportOptions(const ExtractionOpt& opt)
{
	m_extraction_opt = opt;
	m_uiw.export_isos_toggle->setChecked(opt.extract_isos);
	m_uiw.export_pbr_toggle->setChecked(opt.extract_pbr);
	m_uiw.export_stairs_toggle->setChecked(opt.extract_stairs);
	m_uiw.export_smooth_toggle->setChecked(opt.extract_smoothed_stairs);
	m_uiw.export_inter_toggle->setChecked(opt.extract_interpolate);

	ResetExportOptions();
}

void MainAmplificationWindow::DefaultExportOptions()
{
	ChangeExportOptions(ExtractionOpt());
}

///////////////////////////////
////    IMAGES TO EXPORT   ////
///////////////////////////////

// Enregistre la scene actuelle dans une image
void MainAmplificationWindow::ExportSceneImage(const QString& name)
{
	QString finalname = name;
	if (name == "")
	{
		finalname = QFileDialog::getSaveFileName(
			this,
			tr("Save File"),
			"scene.svg",
			tr("SVG Files (*.svg);;All Files (*.*)")
		);
	}

	if (!finalname.isEmpty())
	{
		QGraphicsScene* s = m_uiw.iso_view->scene();
		System::FlipVertical(*s, m_edition_box.GetQtRect());
		System::SaveSvg(s, finalname, m_edition_box.GetQtRect());
	}
}

// Enregistre une image svg des isolignes actuelles
void MainAmplificationWindow::ExportIsosSVG(const QString& name)
{
	if (m_isos.Size() == 0)
	{
		qWarning() << "No isolines yet, impossible to export them";
		return;
	}

	QString finalname = name;
	if (name == "")
	{
		finalname = QFileDialog::getSaveFileName(
			this,
			tr("Save File"),
			"isos.svg",
			tr("SVG Files (*.svg);;All Files (*.*)")
		);
	}

	if (!finalname.isEmpty())
	{
		QGraphicsScene* s = m_isos.ToScene(m_extraction_opt.isos_opt);
		if (m_extraction_opt.isos_white_background)
		{
			auto* item = s->addRect(m_edition_box.GetQtRect(), QPen(Qt::white), QBrush(Qt::white));
			item->setZValue(-10000);
		}
		System::FlipVertical(*s, m_edition_box.GetQtRect());
		System::SaveSvg(s, finalname, m_edition_box.GetQtRect());
	}
}

// Enregistre une image de la version en escalier des isos actuelles
void MainAmplificationWindow::ExportTerrain(const HeightField& hf, const QString& name)
{
	QString finalname = name;
	if (name == "")
	{
		finalname = QFileDialog::getSaveFileName(
			this,
			tr("Save File"),
			"terrain.png",
			tr("PNG Files (*.png);;All Files (*.*)")
		);
	}

	if (!finalname.isEmpty())
	{
		QImage img;
		if (m_extraction_opt.terrain_palette)
			img = hf.CreateImage(*m_extraction_opt.terrain_palette);
		else
			img = hf.CreateImage();

		img.flipped().save(finalname);

	}
}

// Enregistre une image de la version en escalier des isos actuelles
void MainAmplificationWindow::ExportIsosStairs(const QString& name, int max_x, int max_y)
{
	if (m_isos.Size() == 0)
	{
		qWarning() << "No isolines yet, impossible to export the stairs version of them";
		return;
	}

	QString finalname = name;
	if (name == "")
	{
		finalname = QFileDialog::getSaveFileName(
			this,
			tr("Save File"),
			"stairs.png",
			tr("PNG Files (*.png);;All Files (*.*)")
		);
	}

	if (!finalname.isEmpty())
	{
		IsoLineTerrain ilt(m_isos);
		int x, y;
		get_pixel_sizes_for_box(m_generation_box, max_x, max_y, x, y);
		HeightField hf = ilt.StairsField(m_uiw.iso_view->ViewRect(), x, y);
		ExportTerrain(hf, finalname);
	}
}

// Enregistre une image de la version en escalier smmoothed des isos actuelles
void MainAmplificationWindow::ExportIsosSmoothStairs(const QString& name, int max_x, int max_y, int nb_smooth)
{
	if (m_isos.Size() == 0)
	{
		qWarning() << "No isolines yet, impossible to export the smoothed stairs version of them";
		return;
	}

	QString finalname = name;
	if (name == "")
	{
		finalname = QFileDialog::getSaveFileName(
			this,
			tr("Save File"),
			"smooth_stairs.png",
			tr("PNG Files (*.png);;All Files (*.*)")
		);
	}

	if (!finalname.isEmpty())
	{
		IsoLineTerrain ilt(m_isos);
		int x, y;
		get_pixel_sizes_for_box(m_generation_box, max_x, max_y, x, y);
		HeightField hf = ilt.SmoothStairsField(m_uiw.iso_view->ViewRect(), x, y);
		ExportTerrain(hf, finalname);
	}
}

// Enregistre une image de la version en escalier des isos actuelles
void MainAmplificationWindow::ExportIsosInterpolate(const QString& name, int max_x, int max_y)
{
	if (m_isos.Size() == 0)
	{
		qWarning() << "No isolines yet, impossible to export the interpolate version of them";
		return;
	}

	QString finalname = name;
	if (name == "")
	{
		finalname = QFileDialog::getSaveFileName(
			this,
			tr("Save File"),
			"interpolate.png",
			tr("PNG Files (*.png);;All Files (*.*)")
		);
	}

	if (!finalname.isEmpty())
	{
		IsoLineTerrain ilt(m_isos);
		int x, y;
		get_pixel_sizes_for_box(m_generation_box, max_x, max_y, x, y);
		HeightField hf = ilt.InterpolateField(m_uiw.iso_view->ViewRect(), x, y);
		ExportTerrain(hf, finalname);
	}
}

// Enregistre une image du terrain en version rendue CG (MSE + PBR)
void MainAmplificationWindow::ExportMSEAndPBRRender(const QString& name, int interpolate_size, int n, double e, double d)
{
	if (m_isos.Size() == 0)
	{
		qWarning() << "No isolines yet, impossible to export the pbr render version of them";
		return;
	}

	QString finalname = name;
	if (name == "")
	{
		finalname = QFileDialog::getSaveFileName(
			this,
			tr("Save File"),
			"pbr.png",
			tr("PNG Files (*.png);;All Files (*.*)")
		);
	}

	if (finalname.isEmpty())
		return;

	int x, y;
	get_pixel_sizes_for_box(m_generation_box, interpolate_size, interpolate_size, x, y);
	IsoLineTerrain ilt(m_isos);
	m_hf = ilt.InterpolateField(m_edition_box, x, y, Norm(m_edition_box.Diagonal()) / 50);
	MSE(n, e, d);
	UpdateGeometry();

	RenderOpt opt = render_basic_opt(m_isos, m_hf);
	ExportPBRRender(opt, finalname);
}

// Enregistre une image du terrain actuel (meme sans iso) en version rendue CG (PBR only)
void MainAmplificationWindow::ExportPBRRender(const RenderOpt& opt, const QString& name)
{
	QString finalname = name;
	if (name == "")
	{
		finalname = QFileDialog::getSaveFileName(
			this,
			tr("Save File"),
			"pbr.png",
			tr("PNG Files (*.png);;All Files (*.*)")
		);
	}

	if (finalname.isEmpty())
		return;

	m_meshWidget->PBR_Render(opt).save(finalname);
	//m_meshWidget->SaveScreen(1920, 1080, finalname);
}

// Enregistre un histogramme donné
void MainAmplificationWindow::ExportHistogram(const HistogramD& histo, const QString& name)
{
	QString finalname = name;
	if (name == "")
	{
		finalname = QFileDialog::getSaveFileName(
			this,
			tr("Save File"),
			"histo.svg",
			tr("SVG Files (*.svg);;All Files (*.*)")
		);
	}

	if (finalname.isEmpty())
		return;

	QGraphicsScene* s = new QGraphicsScene;
	histo.Draw(*s, *m_extraction_opt.histogram_palette, m_extraction_opt.histogram_width, m_extraction_opt.histogram_height);
	System::FlipVertical(*s);
	System::SaveSvg(s, finalname);
}

// Enregistre l'histogramme utilisé pour la génération
void MainAmplificationWindow::ExportHistogram(const QString& name)
{
	ExportHistogram(m_generation_histogram, name);
}


// Enregistre le masque utilisé pour la génération
void MainAmplificationWindow::ExportMaskSVG(const QString& name)
{
	QString finalname = name;
	if (name == "")
	{
		finalname = QFileDialog::getSaveFileName(
			this,
			tr("Save File"),
			"mask.svg",
			tr("SVG Files (*.svg);;All Files (*.*)")
		);
	}

	if (finalname.isEmpty())
		return;

	QGraphicsScene* scene = isolines_mask_scene(m_generation_mask, m_extraction_opt.mask_palette, m_extraction_opt.mask_white_background, m_extraction_opt.mask_black_border);
	System::FlipVertical(*scene, m_generation_box.GetQtRect());
	System::SaveSvg(scene, finalname, m_generation_box.GetQtRect());
}

void MainAmplificationWindow::ExportMask(const QString& name)
{
	QString finalname = name;
	if (name == "")
	{
		finalname = QFileDialog::getSaveFileName(
			this,
			tr("Save File"),
			"mask.png",
			tr("PNG Files (*.png);;All Files (*.*)")
		);
	}

	if (finalname.isEmpty())
		return;

	m_generation_mask.CreateImage().flipped().save(finalname);
}

// Enregistre le masque utilisé pour la génération
void MainAmplificationWindow::ExportNoise(const QString& name, int max_x, int max_y)
{
	QString finalname = name;
	if (name == "")
	{
		finalname = QFileDialog::getSaveFileName(
			this,
			tr("Save File"),
			"noise.png",
			tr("PNG Files (*.png);;All Files (*.*)")
		);
	}

	if (finalname.isEmpty())
		return;

	int x, y;
	get_pixel_sizes_for_box(m_generation_box, max_x, max_y, x, y);

	QImage image;
	if (m_extraction_opt.noise_inside_iso)
	{
		IsoLines isos = isolines_mask(m_generation_mask);
		image = proba_inside_isos(m_generation_noise, isos, *m_extraction_opt.noise_palette).flipped().scaled(x, y, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	}
	else
	{
		image = m_generation_noise.CreateImage(*m_extraction_opt.noise_palette);
	}
	image.save(finalname);
}

void MainAmplificationWindow::ExportGraph(const GraphPoisson& graph, const QString& name)
{
	if (graph.Size() == 0)
	{
		qWarning() << "Graph empty, impossible to export it.";
		return;
	}

	QString finalname = name;
	if (name == "")
	{
		finalname = QFileDialog::getSaveFileName(
			this,
			tr("Save File"),
			"graph.svg",
			tr("SVG Files (*.svg);;All Files (*.*)")
		);
	}

	if (!finalname.isEmpty())
	{
		Box2 b = Box2(m_generation_box, graph.GetBox());
		QGraphicsScene* s = graph.ToScene(m_extraction_opt.graph_opt);
		if (m_extraction_opt.graph_white_background)
		{
			auto* item = s->addRect(b.GetQtRect(), QPen(Qt::white), QBrush(Qt::white));
			item->setZValue(-10000);
		}
		System::FlipVertical(*s, b.GetQtRect());
		System::SaveSvg(s, finalname, b.GetQtRect());
	}
}

// Enregistre le graphe des zones de la génération
void MainAmplificationWindow::ExportGraphZones(const QString& name)
{
	ExportGraph(m_generation_graph_zones, name);
}

// Enregistre le graphe des zones de la génération, mais comme x graphes différents (donc génère x images)
void MainAmplificationWindow::ExportGraphZonesSplit(const QString& name)
{
	if (m_generation_graph_zones.Size() == 0)
	{
		qWarning() << "No zones graph yet, impossible to export it.";
		return;
	}

	QString dirname = name;
	if (name == "")
	{
		dirname = QFileDialog::getExistingDirectory(
			this,
			tr("Select Directory"),
			QString(),                // default path
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
		);
	}

	if (!dirname.isEmpty())
	{
		GraphPoisson::DisplayOptions opt_copy = m_extraction_opt.graph_opt;
		m_extraction_opt.graph_opt.displayMaxValNodes = false;
		m_extraction_opt.graph_opt.inRangeBorderOutlineRatio = m_extraction_opt.graph_opt.outlineRatio;
		m_extraction_opt.graph_opt.maxColoredVal = -1;

		ExportGraphZones(dirname + "zones.svg");

		int count = 0;
		double prev = 0;

		for (double v : m_generation_graph_zones.SortedValues())
		{
			m_extraction_opt.graph_opt.minDisplayedVal = prev;
			m_extraction_opt.graph_opt.maxDisplayedVal = v + 0.000001;
			prev = m_extraction_opt.graph_opt.minDisplayedVal;
			ExportGraphZones(dirname + "zone-" + QString::number(count) + ".svg");
			count++;

		}

		m_extraction_opt.graph_opt = opt_copy;
	}
}

// Enregistre le graphe des probas de la génération
void MainAmplificationWindow::ExportGraphProba(const QString& name)
{
	ExportGraph(m_generation_graph_proba, name);
}

// Enregistre le graphe résultat de la génération (sans être passé dans l'histo)
void MainAmplificationWindow::ExportGraphResult(const QString& name)
{
	ExportGraph(m_generation_graph_result, name);
}

// Enregistre le graphe résultat, mais en fonction des zones (donc génère x images)
void MainAmplificationWindow::ExportGraphResultSplit(const QString& name)
{
	if (m_generation_graph_result.Size() == 0 || m_generation_graph_zones.Size() == 0)
	{
		qWarning() << "No zones/result graph yet, impossible to export.";
		return;
	}
	if (m_generation_graph_result.Size() != m_generation_graph_zones.Size())
	{
		qWarning() << "zones/result graphs don't have the same size, impossible to export.";
		return;
	}

	QString dirname = name;
	if (name == "")
	{
		dirname = QFileDialog::getExistingDirectory(
			this,
			tr("Select Directory"),
			QString(),                // default path
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
		);
	}

	if (!dirname.isEmpty())
	{
		GraphPoisson::DisplayOptions opt_copy = m_extraction_opt.graph_opt;
		m_extraction_opt.graph_opt.displayMaxValNodes = false;
		m_extraction_opt.graph_opt.inRangeBorderOutlineRatio = m_extraction_opt.graph_opt.outlineRatio;

		ExportGraphResult(dirname + "results.svg");

		int count = 0;
		int n = m_generation_graph_result.Size();

		for (double v : m_generation_graph_zones.SortedValues())
		{
			double min = n;
			double max = 0;
			for (int i = 0; i < n; ++i)
			{
				if (m_generation_graph_zones.At(i) == v)
				{
					Math::SetMinMax(m_generation_graph_result.At(i), min, max);
				}
			}

			m_extraction_opt.graph_opt.minDisplayedVal = min;
			m_extraction_opt.graph_opt.maxDisplayedVal = max;;
			ExportGraphResult(dirname + "result-" + QString::number(count) + ".svg");
			count++;
		}

		m_extraction_opt.graph_opt = opt_copy;
	}
}

// Enregistre le graphe résultat de la génération (sans être passé dans l'histo) en plein d'image pour faire un gif animé de la génération
// Create one gif per zone.
// TODO: Qt cannot create a gif directly
void MainAmplificationWindow::ExportGraphResultGif(const QString& name, int step)
{
	if (m_generation_graph_result.Size() == 0 || m_generation_graph_zones.Size() == 0)
	{
		qWarning() << "No zones/result graph yet, impossible to export.";
		return;
	}
	if (m_generation_graph_result.Size() != m_generation_graph_zones.Size())
	{
		qWarning() << "zones/result graphs don't have the same size, impossible to export.";
		return;
	}

	QString dirname = name;
	if (name == "")
	{
		dirname = QFileDialog::getExistingDirectory(
			this,
			tr("Select Directory"),
			QString(),                // default path
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
		);
	}

	if (!dirname.isEmpty())
	{
		GraphPoisson::DisplayOptions old_opt = m_extraction_opt.graph_opt;

		int n = m_generation_graph_result.Size();
		int zone = 1;
		int id = 0;

		for (double v : m_generation_graph_zones.SortedValues())
		{
			double min = n;
			double max = 0;
			for (int i = 0; i < n; ++i)
			{
				if (Math::Abs(m_generation_graph_zones.At(i) - v) <= 0.0001)
				{
					Math::SetMinMax(m_generation_graph_result.At(i), min, max);
				}
			}

			m_extraction_opt.graph_opt.minDisplayedVal = min - 0.0001;
			m_extraction_opt.graph_opt.maxDisplayedVal = max + 0.0001;

			for (int i = 0; i <= n; i += step)
			{
				m_extraction_opt.graph_opt.minColoredVal = -1;
				m_extraction_opt.graph_opt.maxColoredVal = i - 1;
				ExportGraphResult(dirname + "zone-" + QString::number(zone) + "-step-" + QString::number(i) + ".svg");
			}
			zone++;
		}

		m_extraction_opt.graph_opt = old_opt;
	}
}

// Enregistre le graphe résultat de la génération après être passé dans l'histo
void MainAmplificationWindow::ExportGraphHeights(const QString& name)
{
	ExportGraph(m_generation_graph_heights, name);
}

// Enregistre x graphes correspondant à l'extraction de chaque zones
void MainAmplificationWindow::ExportGraphsEachHeight(const QString& name)
{
	if (m_generation_graph_heights.Size() == 0)
	{
		qWarning() << "No zones/result graph yet, impossible to export.";
		return;
	}

	QString dirname = name;
	if (name == "")
	{
		dirname = QFileDialog::getExistingDirectory(
			this,
			tr("Select Directory"),
			QString(),                // default path
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
		);
	}

	if (!dirname.isEmpty())
	{
		GraphPoisson::DisplayOptions old_opt = m_extraction_opt.graph_opt;

		int count = 1;
		int id = 0;

		QVector<double> heights = m_generation_graph_heights.SortedValues();
		m_extraction_opt.graph_opt.minColoredVal = heights[0] - 1;
		m_extraction_opt.graph_opt.maxColoredVal = heights[0] - 1;
		m_extraction_opt.graph_opt.inRangeBorderOutlineRatio = m_extraction_opt.graph_opt.outlineRatio;
		m_extraction_opt.graph_opt.displayMaxSpecificColor = false;
		ExportGraphHeights(dirname + "height-0.svg");

		for (double v : heights)
		{
			m_extraction_opt.graph_opt.maxColoredVal = v;
			ExportGraphHeights(dirname + "height-" + QString::number(count) + ".svg");
			count++;
		}

		m_extraction_opt.graph_opt = old_opt;
	}
}

void MainAmplificationWindow::ExportGraphWithoutInf(const GraphPoisson& graph, const QString& name)
{
	GraphPoisson::DisplayOptions opt_copy = m_extraction_opt.graph_opt;

	// To avoid the value by default put on graphs
	if (m_extraction_opt.graph_opt.maxVal == Math::Infinity)
	{
		m_extraction_opt.graph_opt.maxVal = -Math::Infinity;
		for (int i = 0; i < graph.Size(); ++i)
			if (graph.At(i) != Math::Infinity)
				m_extraction_opt.graph_opt.maxVal = Math::Max(m_extraction_opt.graph_opt.maxVal, graph.At(i));

		m_extraction_opt.graph_opt.maxDisplayedVal = m_extraction_opt.graph_opt.maxVal;
	}

	ExportGraph(graph, name);

	m_extraction_opt.graph_opt = opt_copy;
}

// Enregistre le graphe eden asc de la génération
void MainAmplificationWindow::ExportGraphEdenAsc(const QString& name, int zone)
{
	if (m_generation_graph_eden_asc.size() <= zone || m_generation_graph_eden_asc[zone].Size() == 0)
	{
		qWarning() << "No eden asc graph for zone" << zone << "yet, impossible to export it.";
		return;
	}

	ExportGraphWithoutInf(m_generation_graph_eden_asc[zone], name);
}

// Enregistre le graphe eden desc de la génération
void MainAmplificationWindow::ExportGraphEdenDesc(const QString& name, int zone)
{
	if (m_generation_graph_eden_desc.size() <= zone || m_generation_graph_eden_desc[zone].Size() == 0)
	{
		qWarning() << "No eden desc graph for zone" << zone << "yet, impossible to export it.";
		return;
	}

	ExportGraphWithoutInf(m_generation_graph_eden_desc[zone], name);
}

// Enregistre le graphe eden desc + asc final de la génération
void MainAmplificationWindow::ExportGraphEdenFinal(const QString& name, int zone)
{
	if (m_generation_graph_eden_final.size() <= zone || m_generation_graph_eden_final[zone].Size() == 0)
	{
		qWarning() << "No final eden graph for zone" << zone << "yet, impossible to export it.";
		return;
	}

	ExportGraphWithoutInf(m_generation_graph_eden_final[zone], name);
}

// Enregistre les graphes d'eden asc et desc
void MainAmplificationWindow::ExportGraphsDoubleEdenGif(const QString& name, int step)
{
	// TODO add warnings for graphs not existing/not the same size

	QString dirname = name;
	if (name == "")
	{
		dirname = QFileDialog::getExistingDirectory(
			this,
			tr("Select Directory"),
			QString(),                // default path
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
		);
	}

	if (!dirname.isEmpty())
	{
		int n = m_generation_graph_eden_desc.size();
		GraphPoisson::DisplayOptions opt_copy = m_extraction_opt.graph_opt;

		static const Color Orange(247, 150, 70);
		static const Color Grey = Color(244, 244, 244);
		static const Color Green(146, 208, 80);
		Palette* paletteAsc = new Palette({ Green, Grey });
		Palette* paletteDesc = new Palette({ Orange, Grey });
		Palette* paletteFinal = new Palette({ Green, Grey, Orange });

		m_extraction_opt.graph_opt.inRangeBorderOutlineRatio = m_extraction_opt.graph_opt.outlineRatio;

		for (int i = 0; i < n; ++i)
		{
			int zone_n = m_generation_graph_eden_final[i].Size();

			m_extraction_opt.graph_opt.palette = paletteAsc;
			for (int j = 0; j <= zone_n; j += step)
			{
				m_extraction_opt.graph_opt.minColoredVal = -1;
				m_extraction_opt.graph_opt.maxColoredVal = j - 1;
				ExportGraphEdenAsc(dirname + "asc-" + QString::number(i + 1) + "_" + QString::number(j) + ".svg", i);
			}

			m_extraction_opt.graph_opt.palette = paletteDesc;
			for (int j = 0; j <= zone_n; j += step)
			{
				m_extraction_opt.graph_opt.minColoredVal = -1;
				m_extraction_opt.graph_opt.maxColoredVal = j - 1;
				ExportGraphEdenDesc(dirname + "desc-" + QString::number(i + 1) + "_" + QString::number(j) + ".svg", i);
			}

			m_extraction_opt.graph_opt.palette = paletteFinal;
			ExportGraphEdenFinal(dirname + "final-" + QString::number(i + 1) + ".svg", i);
		}

		delete paletteAsc;
		delete paletteDesc;
		delete paletteFinal;

		m_extraction_opt.graph_opt = opt_copy;
	}
}

// Enregistre toutes les images demandées (ui), derrière le prefix
void MainAmplificationWindow::ExportAll(const QString& prefix)
{
	if (m_extraction_opt.extract_isos)
		ExportIsosSVG(prefix + "_isos.svg");
	if (m_extraction_opt.extract_pbr)
		ExportMSEAndPBRRender(prefix + "_pbr.png", m_extraction_opt.pbr_interpolate_size, m_extraction_opt.pbr_mse_iter, m_extraction_opt.pbr_mse_erosion, m_extraction_opt.pbr_mse_deposition);
	if (m_extraction_opt.extract_stairs)
		ExportIsosStairs(prefix + "_stairs.png", m_extraction_opt.stairs_x, m_extraction_opt.stairs_y);
	if (m_extraction_opt.extract_smoothed_stairs)
		ExportIsosSmoothStairs(prefix + "_smooth_stairs.png", m_extraction_opt.smoothed_stairs_x, m_extraction_opt.smoothed_stairs_y, m_extraction_opt.smoothed_stairs_nb_smooth);
	if (m_extraction_opt.extract_interpolate)
		ExportIsosInterpolate(prefix + "_interpolate.png", m_extraction_opt.interpolate_x, m_extraction_opt.interpolate_y);
	if (m_extraction_opt.extract_histogram)
		ExportHistogram(prefix + "_histogram.svg");
	if (m_extraction_opt.extract_mask_svg)
		ExportMaskSVG(prefix + "_mask.svg");
	if (m_extraction_opt.extract_mask)
		ExportMask(prefix + "_mask.png");
	if (m_extraction_opt.extract_noise)
		ExportNoise(prefix + "_noise.png", m_extraction_opt.noise_x, m_extraction_opt.noise_y);
	if (m_extraction_opt.extract_graph_zones)
		ExportGraphZones(prefix + "_graph_zones.png");
	if (m_extraction_opt.extract_graph_proba)
		ExportGraphProba(prefix + "_graph_proba.png");
	if (m_extraction_opt.extract_graph_result)
		ExportGraphResult(prefix + "_graph_result.png");
}

///////////////////////////////
////        ACTIONS        ////
///////////////////////////////

// Construit le graphe en fonction du masque et du radius
void MainAmplificationWindow::CreateGenerationZones()
{
	m_generation_graph_zones = GraphPoisson(m_generation_mask, m_generation_radius);
	m_generation_graph_zones.SetStrictValueFromScalarField(m_generation_mask);
	qDebug() << "[Zones graph] nb particles:" << m_generation_graph_zones.Size();
}

// Reset les valeurs du graph de zones, lorsque le masque est le même que précédemment, mais avec différentes zones
void MainAmplificationWindow::ResetGenerationZones()
{
	m_generation_graph_zones.SetStrictValueFromScalarField(m_generation_mask);
	qDebug() << "[Zones graph] nb particles:" << m_generation_graph_zones.Size();
}

void MainAmplificationWindow::GenerateV1(int debug, const QString& root)
{
	IsoVectoGenerationV1 gen(m_generation_graph_zones, m_generation_noise);
	gen.Generate(debug, root);
	m_generation_graph_result = gen.Result();
	m_generation_graph_proba = gen.Proba();
}

void MainAmplificationWindow::GenerateV2(int debug, const QString& root)
{
	IsoVectoGenerationV2 gen(m_generation_graph_zones, m_generation_noise);
	gen.Generate(debug, root);
	m_generation_graph_result = gen.Result();
	m_generation_graph_proba = gen.Proba();
}

void MainAmplificationWindow::GenerateV3(int debug, const QString& root)
{
	IsoVectoGenerationV3 gen(m_generation_graph_zones, m_generation_noise);
	gen.Generate(debug, root);
	m_generation_graph_result = gen.Result();
	m_generation_graph_proba = gen.Proba();
	m_generation_graph_eden_asc = gen.EdenAsc();
	m_generation_graph_eden_desc = gen.EdenDesc();
	m_generation_graph_eden_final = gen.EdenFinal();
}

void MainAmplificationWindow::GenerateV3Endo(int debug, const QString& root)
{
	IsoVectoGenerationV3 gen(m_generation_graph_zones, m_generation_noise, true);
	gen.Generate(debug, root);
	m_generation_graph_result = gen.Result();
	m_generation_graph_proba = gen.Proba();
}

// Applique la génération en fonction des paramètres donnés
void MainAmplificationWindow::Generate(int version)
{
	// Creation of zones
	QElapsedTimer timer;
	timer.start();

	qDebug() << "Graph creation...";
	
	CreateGenerationZones();

	qint64 t1 = timer.elapsed();
	qDebug() << t1 << "ms";

	// Assignment of heights = Eden growth
	qDebug() << "Heights assignments...";

	switch (version)
	{
	case 1:
		GenerateV1();
		break;
	case 2:
		GenerateV2();
		break;
	case 3:
		GenerateV3();
		break;
	case 4:
		GenerateV3Endo();
		break;
	default:
		qDebug() << "Iso Generation Version" << version << "does not exists.";
		break;
	}

	qint64 t2 = timer.elapsed();
	qDebug() << t2 - t1 << "ms";

	// Extraction of isolines
	qDebug() << "Isolines extraction...";

	SetIsosFromHistogram();

	qint64 t3 = timer.elapsed();
	qDebug() << t3 - t2 << "ms";

	// UI informations
	m_test_ms_taken = t3;
	m_uiw.timing_text->setText(QString::number(m_test_ms_taken) + " ms");
	m_uiw.particles_text->setText(QString::number(m_generation_graph_result.Size()) + " particles");
}

// Change le terrain en fonction des isos
void MainAmplificationWindow::RecomputeTerrainFromIsos(bool resetView)
{
	if (!m_isos.IsEmpty())
	{
		IsoLineTerrain ilt(m_isos);
		switch (m_terrain_type)
		{
		case STAIRS:
			m_hf = ilt.StairsField(m_edition_box, m_terrain_x, m_terrain_y);
			break;
		case INTERPOLATE:
			m_hf = ilt.InterpolateField(m_edition_box, m_terrain_x, m_terrain_y);
			break;
		case SMOOTH_STAIRS:
			m_hf = ilt.SmoothStairsField(m_edition_box, m_terrain_x, m_terrain_y);
			break;
		case ERODED:
			m_hf = ilt.InterpolateField(m_edition_box, m_terrain_x, m_terrain_y);
			MSE();
			break;
		case HEAT:
			m_hf = ilt.HeatField(m_edition_box, m_terrain_x, m_terrain_y, true /* check with false*/);
			break;
		}

		//int i = 0;
		//m_meshWidget->ClearAll();
		//for (Mesh& m : ilt.GetMesh())
		//{
		//	QString name = "iso " + QString::number(i);
		//	m_meshWidget->AddMesh(name, m);
		//	m_meshWidget->MeshWidget::UseWireframe(name, true);
		//	++i;
		//}
		//m_hf = HeightField();

		if (resetView)
		{
			ResetCamera();
		}
		UpdateGeometry();
	}
}

// Change les isos
void MainAmplificationWindow::SetIsos(const IsoLines& newIsos, bool resetView, bool saveInHistoric)
{
	m_isos = newIsos;
	emit isoChanged(&m_isos);
	if (m_isos.Size() != 0)
		m_isos.ChangeHeightsMinMax(m_min_height, m_max_height);

	m_uiw.iso_view->ChangeIsos(&m_isos);
	if (resetView)
	{
		m_uiw.iso_view->ResetView();
	}
	RecomputeTerrainFromIsos(resetView);

	m_uiw.points_text->setText(QString::number(m_isos.VertexSize()) + " pts");
	m_uiw.expected_points_text->setText("> " + QString::number(Math::Ceil(m_isos.TotalLength() / m_edition_point_min_dist)) + " pts");

	if (saveInHistoric)
		SaveCurrentIsolines();
}

// Change les isos en fonction de l'histogramme choisi, ATTENTION, cela va supprimer toutes les modifications faites
void MainAmplificationWindow::SetIsosFromHistogram()
{
	// Pas encore de génération
	if (m_generation_graph_result.Size() == 0)
		return;

	// Copie pour ne pas perdre les valeurs de progression de la génération
	m_generation_graph_heights = m_generation_graph_result;
	m_generation_graph_heights.SetValueFromHistogram(m_generation_histogram);
	IsoLines isolines(m_generation_graph_heights, m_generation_histogram);
	SetIsos(isolines, true, true);
}

// Change les isos pour être en accord avec le masque actuel
void MainAmplificationWindow::SetIsosFromMask()
{
	IsoLines isos(m_generation_mask, m_uiw.mask_nb_isos_edit->text().toInt());
	SetIsos(isos);
}

// Ajoute une isos � l'ensemble d'iso actuel
void MainAmplificationWindow::AddIso(const Polygon2 poly)
{
	IsoLinePoly ilp(poly, 0);
	ilp.ChangeOrder(true); // in case the polygon is defined in the wrong order
	m_isos.Append(Polygon2(ilp));
	SetIsos(m_isos, false, true);
}

// Change the view with the given polygon and be ready for the generation
void MainAmplificationWindow::ResetViewWithPolygon(const Polygon2& poly)
{
	Box2 b = poly.GetBox().ScaledCentered(1.1);
	ChangeViewBox(b);
	ChangeGenBox(b);
	SetIsos(IsoLines({ poly }), true, true);
	ChangeMaskFromCurrentIsos();
}

// Remove isos from the scene
void MainAmplificationWindow::CleanIsos()
{
	SetIsos({});
	m_uiw.iso_view->ClearView();
	m_uiw.iso_view->SetViewRect(m_edition_box);
}

// Apply MSE on the terrain
void MainAmplificationWindow::MSE(int n, double e, double d)
{
	// MSE does not apply to value below 0, so we have to rescale it.
	double a, b;
	m_hf.GetRange(a, b);
	m_hf.SetRange(0, b - a);
	m_hf.ErosionAmpli(n, e, d);
	m_hf.SetRange(a, b);
	UpdateGeometry();
	//ResetCamera();
}

// Remove every isos with a length < min_dist
void MainAmplificationWindow::RemoveSmallIsos(double min_dist)
{
	QVector<IsoLinePoly> isos;

	for (int i = 0; i < m_isos.Size(); ++i)
	{
		if (m_isos[i].Length() > min_dist)
		{
			isos.append(m_isos[i]);
		}
	}

	SetIsos(IsoLines(isos), false, true);
}

// Resample isos
void MainAmplificationWindow::ResampleIsos(double min_dist)
{
	m_isos.Resample(min_dist);
	SetIsos(m_isos, false, true);
}

// Resample isos with cubic spline
void MainAmplificationWindow::ResampleSplineIsos(double min_dist)
{
	m_isos.ResampleSpline(min_dist);
	SetIsos(m_isos, false, true);
}

void MainAmplificationWindow::SmoothIsos(double dist, double delta, int nb_iter)
{
	IsoLines copy = m_isos;
	copy.Smooth(dist, delta, nb_iter);
	ProtectIsos(copy);
	SetIsos(copy, false, true);
}

void MainAmplificationWindow::SmoothInsidePoint(const Vector2& point)
{
	IsoLines copy = m_isos;
	for (int ind = 0; ind < m_isos.Size(); ++ind)
	{
		copy[ind] = copy[ind].SmoothInsideCircle(Circle2(point, m_edition_radius), m_smoothing_dist, m_smoothing_delta);
	}
	ProtectIsos(copy);
	SetIsos(copy);
}

void MainAmplificationWindow::DisplacementAlongCurve(const QVector<Vector2>& path, double radius, double strength, double pow)
{
	IsoLines copy = m_isos;
	for (int ind = 0; ind < m_isos.Size(); ++ind)
	{
		copy[ind] = copy[ind].DisplacementTowardCurve(PointCurve2(path), radius, strength, pow);
	}
	ProtectIsos(copy);
	SetIsos(copy, false, true);
}

// Bouge les isos actuels
void MainAmplificationWindow::Warp(const QVector<Vector2>& points, double radius, double strength, double pow, double factor)
{
	WarpingIsos warp(m_isos);
	warp.SetPathPoints(points);
	warp.PathDisplacement(radius, strength, pow, factor);
	SetIsos(warp.WarpedIsos(), false, true);
}

void MainAmplificationWindow::ViewPathFinished(const QVector<Vector2>& path)
{
	//cout << "QVector<Vector2> path = {" << endl;
	//for (const Vector2& p: path)
	//{
	//	cout << "\tVector2(" << p[0] << ", " << p[1] << ")," << endl;
	//}
	//cout << "};" << endl;
	if (m_current_tool == DRAWING)
		AddIso(Polygon2(path));
	if (m_current_tool == WARPING)
		Warp(path, m_edition_radius, m_warping_strength, m_warping_pow, m_warping_factor);
	if (m_current_tool == SLOPING)
		DisplacementAlongCurve(path, m_edition_radius, m_sloping_strength, m_sloping_pow);

	if (m_current_tool == SMOOTHING)
		SaveCurrentIsolines();
}

void MainAmplificationWindow::ViewRightClick(const Vector2& point)
{
	if (m_current_tool == SMOOTHING)
		SmoothInsidePoint(point);
	if (m_current_tool == PROTECTING)
		ProtectZone(point);
}

// Donner des isolines à cette méthode pour qu'elle protège les points protégé
void MainAmplificationWindow::ProtectIsos(IsoLines& isos)
{
	if (&isos == &m_isos)
	{
		qDebug() << "You are trying to protect the same isos as the one currently used, it will do nothing. Make a copy before modifying";
		return;
	}

	if (isos.Size() != m_isos.Size())
	{
		qDebug() << "1 You are trying to protect isolines which do not have the same size, this is not possible";
		return;
	}

	for (int ind = 0; ind < m_isos.Size(); ++ind)
	{
		if (m_isos[ind].Size() != isos[ind].Size())
		{
			qDebug() << "2 You are trying to protect isolines which do not have the same size, this is not possible";
			return;
		}
	}

	if (m_protected_zones.empty())
		return;

	for (int ind = 0; ind < m_isos.Size(); ++ind)
	{
		for (int i = 0; i < m_isos[ind].Size(); ++i)
		{
			if (WithinProtectZone(m_isos[ind].Vertex(i)))
			{
				isos[ind].Vertex(i) = m_isos[ind].Vertex(i);
			}
		}
	}
}

// Return true if a point is protected
bool MainAmplificationWindow::WithinProtectZone(const Vector2& point)
{
	for (int i = 0; i < m_protected_zones.size(); ++i)
	{
		Vector2 c = m_protected_zones[i].Center();
		double r = m_protected_zones[i].Radius();
		double d = Norm(c - point);
		if (d < r)
		{
			return true;
		}
	}
	return false;
}

// Add a protection zone
void MainAmplificationWindow::ProtectZone(const Vector2& point)
{
	if (!m_protecting_remove)
	{
		m_protected_zones.append(Circle2(point, m_edition_radius));
	}
	else
	{
		for (int i = 0; i < m_protected_zones.size(); ++i)
		{
			Vector2 c = m_protected_zones[i].Center();
			double r = m_protected_zones[i].Radius();
			double d = Norm(c - point);
			if (d < m_edition_radius + r)
			{
				m_protected_zones[i] = m_protected_zones.back();
				m_protected_zones.pop_back();
				i--;
			}
		}
	}
	m_uiw.iso_view->SetProtectedZones(&m_protected_zones);
}

// Protect every small isolines
void MainAmplificationWindow::ProtectSmallIsolines(double max_length)
{
	for (int i = 0; i < m_isos.Size(); ++i)
	{
		if (m_isos[i].Length() < max_length)
		{
			ProtectIsoline(i);
		}
	}
}

// Protect one specific iso
// TODO: faire en sorte que les isos protégés soit pas fait comme des cercles
void MainAmplificationWindow::ProtectIsoline(int id)
{
	if (id >= m_isos.Size())
	{
		qDebug() << "Isoline" << id << "does not exist";
		return;
	}

	int n = m_isos[id].Size();
	for (int i = 0; i < n; ++i)
	{
		Vector2 e1 = m_isos[id].Edge((i + n - 1) % n);
		Vector2 e2 = m_isos[id].Edge(i);
		double r = Math::Max(Norm(e1), Norm(e2));
		m_protected_zones.append(Circle2(m_isos[id].Vertex(i), r));
	}
	m_uiw.iso_view->SetProtectedZones(&m_protected_zones);
}

// Delete every protection zones
void MainAmplificationWindow::CleanProtectedZones()
{
	m_protected_zones.clear();
	m_uiw.iso_view->SetProtectedZones(&m_protected_zones);
}

void MainAmplificationWindow::RemoveIsolines(const QSet<int>& toRemove, bool recursive)
{
	m_isos.Remove(toRemove, recursive);
	SetIsos(m_isos, false, true);
}

void MainAmplificationWindow::RemoveSmallIsolines(double max_length)
{
	QSet<int> toRemove;
	for (int i = 0; i < m_isos.Size(); ++i)
		if (m_isos[i].Length() <= max_length)
			toRemove.insert(i);

	RemoveIsolines(toRemove);
}

void MainAmplificationWindow::RemoveSelectedIsolines()
{
	RemoveIsolines(m_uiw.iso_view->GetSelectedIsos());
}

void MainAmplificationWindow::SaveCurrentIsolines()
{
	m_isos_historic = m_isos_historic.mid(0, m_current_historic_index + 1);
	m_isos_historic.append(m_isos);
	m_current_historic_index++;
}

void MainAmplificationWindow::Undo()
{
	if (m_current_historic_index > 0)
	{
		m_current_historic_index--;
		SetIsos(m_isos_historic[m_current_historic_index]);
	}
}

void MainAmplificationWindow::Redo()
{
	if (m_current_historic_index < m_isos_historic.size() - 1)
	{
		m_current_historic_index++;
		SetIsos(m_isos_historic[m_current_historic_index]);
	}
}