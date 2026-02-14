#pragma once

#include "uic_interface.h" 
#include "realtime.h" 
#include "heightfield.h"
#include "displacement-function.h"
#include "histogramd.h"

typedef TerrainRaytracingWidget::PBR_Render_Options RenderOpt;

class MainAmplificationWindow : public QMainWindow {
	Q_OBJECT

  private:
	struct ExtractionOpt
	{
		ExtractionOpt()
		{
			isos_opt.fill = true;
		}

		bool extract_isos = true;
		bool extract_pbr = true;
		bool extract_stairs = true;
		bool extract_smoothed_stairs = true;
		bool extract_interpolate = true;

		bool extract_histogram = true;
		bool extract_mask_svg = true;
		bool extract_mask = false;
		bool extract_noise = true;

		bool extract_graph_zones = false;
		bool extract_graph_proba = false;
		bool extract_graph_result = false;

		// Params for isos
		IsoLines::DisplayOptions isos_opt;
		bool isos_white_background = true;

		// Params for isos terrain
		int stairs_x = 1024;
		int stairs_y = 1024;
		int smoothed_stairs_x = 1024;
		int smoothed_stairs_y = 1024;
		int smoothed_stairs_nb_smooth = 1;
		int interpolate_x = 1024;
		int interpolate_y = 1024;
		AnalyticPalette* terrain_palette = nullptr;

		// Params for pbr
		int pbr_mse_iter = 3;
		int pbr_interpolate_size = 256;
		double pbr_mse_erosion = 1;
		double pbr_mse_deposition = 1;

		// Params for histo
		GenericPalette* histogram_palette = new Palette({ Qt::white });
		double histogram_width = 1000;
		double histogram_height = 500;

		// Params for noise
		GenericPalette* noise_palette = new AnalyticPalette(0);
		int noise_x = -1;
		int noise_y = -1;
		bool noise_inside_iso = true;

		// Params for mask
		GenericPalette* mask_palette = new Palette({Color(216, 204, 200), Color(189, 171, 164), Color(142, 110, 99), Color(123, 85, 71), Color(95, 64, 56)});
		bool mask_black_border = false;
		bool mask_white_background = true;

		// Params for graphs
		GraphPoisson::DisplayOptions graph_opt;
		bool graph_white_background = true;
	};

  public:
	MainAmplificationWindow();
	~MainAmplificationWindow();

	void keyPressEvent(QKeyEvent* event) override;

	void CreateActions();

	void UpdateGeometry(bool entire = true);

  signals:
	void isoChanged(IsoLines*);

  public slots:
	// HeightField processing
	void SmoothHeightField();
	void ScaleButton();

	// Figures
	void GenerateFigures();
	void GenerateFigure1(const QString&);
	void GenerateFigure2(const QString&);
	void GenerateFigure5(const QString&);
	void GenerateFigure6And8(const QString&);
	void GenerateFigure7(const QString&);
	void GenerateFigure10(const QString&);
	void GenerateFigure11(const QString&);
	void GenerateFigure12(const QString&);
	void GenerateFigure13And14(const QString&);
	void GenerateFigure16And19And20(const QString&);
	void GenerateFigure21(const QString&);
	void GenerateFigure22(const QString&);
	void GenerateFigure23(const QString&);
	void GenerateFigure24(const QString&);
	void GenerateFigure25AndTable1(const QString&);
	void GenerateFigure26(const QString&);
	void GenerateFigure27(const QString&);
	void GenerateFigure28(const QString&);

	void GenerateFigurePresentation1(const QString&);
	void GenerateFigurePresentation2(const QString&);
	void GenerateFigurePresentation3(const QString&);
	void GenerateFigurePresentation4(const QString&);
	void GenerateFigurePresentation5(const QString&);
	void GenerateFigurePresentation6(const QString&);

	// Parameters
	void ResetHistogramView();
	void ChangeHistogram(const HistogramD&);
	void ChangeHistogramFromFile(const QString&, int);
	void ChangeHistogramFromFunction(int, int);
	void ChangeHistogramFromUI();
	void BrowseHistogram();

	void ResetNoiseView();
	void ChangeNoise(const ScalarField2&, double = 1);
	void ChangeNoiseFromFile(const QString&, double = 1);
	void ChangeNoisePower(double);
	void BrowseNoise();

	void ResetMaskView();
	void ChangeMask(const ScalarField2&);
	void ChangeMaskFromFile(const QString&, int = 1, double = 0.01, int smooth_number = 7);
	void ChangeMaskFromMaskFile(const QString&);
	void ChangeMaskParams(int, double = 0.01);
	void ChangeMaskFromIsos(const IsoLines& isos);
	void ChangeMaskFromCurrentIsos();
	void BrowseMask();

	void ResetPoissonRadius();
	void ChangeEstimatedNbParticles(int n);

	void ResetGenBox();
	void ChangeGenBox(const Box2&);
	void ResetViewBox();
	void ChangeViewBox(const Box2&);
	void ChangeBoxes(const Box2&);

	void ResetHeights();
	void ChangeHeights(double, double);
	void ResetTerrainSize();
	void ChangeTerrainSize(double, double);

	void ResetCamera();
	void ChangeCamera(double, double, double = -1, const Vector & = Vector(0));
	void ChangeCamera(const Camera&);
	void DefaultCamera();
	void LookDownCamera();

	// TODO see if we need setters
	void ResetEditionOptions();
	void ResetSmoothingOptions();
	void ResetWarpingOptions();
	void ResetSlopingOptions();
	void ResetProtectingOptions();
	void ChangeEditionTool(int);

	void ChangeEditionRadius(double);
	void ChangeEditionResamplingMinDist(double);
	void ChangeSmoothingDistOption(double);
	void ChangeSmoothingDeltaOption(double);
	void ChangeProtectionMaxLength(double);

	void ResetExportOptions();
	void ChangeExportOptions(const ExtractionOpt& opt);
	void DefaultExportOptions();

	// Images to export
	void ExportSceneImage(const QString& = "");
	void ExportIsosSVG(const QString& = "");
	void ExportTerrain(const HeightField&, const QString& = "");
	void ExportIsosStairs(const QString& = "", int = 256, int = 256);
	void ExportIsosSmoothStairs(const QString& = "", int = 256, int = 256, int = 1);
	void ExportIsosInterpolate(const QString& = "", int = 256, int = 256);
	void ExportMSEAndPBRRender(const QString & = "", int = 256, int = 3, double = 1, double = 1);
	void ExportPBRRender(const RenderOpt&, const QString& = "");
	void ExportHistogram(const HistogramD&, const QString & = "");
	void ExportHistogram(const QString & = "");
	void ExportMaskSVG(const QString & = "");
	void ExportMask(const QString & = "");
	void ExportNoise(const QString & = "", int = 256, int = 256);
	void ExportGraph(const GraphPoisson&, const QString & = "");
	void ExportGraphZones(const QString & = "");
	void ExportGraphZonesSplit(const QString & = "");
	void ExportGraphProba(const QString & = "");
	void ExportGraphResult(const QString & = "");
	void ExportGraphResultSplit(const QString & = "");
	void ExportGraphResultGif(const QString & = "", int = 1);
	void ExportGraphHeights(const QString & = "");
	void ExportGraphsEachHeight(const QString & = "");
	void ExportGraphWithoutInf(const GraphPoisson&, const QString & = "");
	void ExportGraphEdenAsc(const QString & = "", int = 0);
	void ExportGraphEdenDesc(const QString & = "", int = 0);
	void ExportGraphEdenFinal(const QString & = "", int = 0);
	void ExportGraphsDoubleEdenGif(const QString & = "", int = 1);
	void ExportAll(const QString & = "");

	// Actions
	void CreateGenerationZones();
	void ResetGenerationZones();
	void GenerateV1(int = 0, const QString& = "");
	void GenerateV2(int = 0, const QString& = "");
	void GenerateV3(int = 0, const QString & = "");
	void GenerateV3Endo(int = 0, const QString& = "");
	void Generate(int version = 3);

	void RecomputeTerrainFromIsos(bool = false);
	void SetIsos(const IsoLines&, bool = false, bool = false);
	void SetIsosFromHistogram();
	void SetIsosFromMask();
	void AddIso(const Polygon2);
	void ResetViewWithPolygon(const Polygon2&);
	void CleanIsos();
	void MSE(int = 3, double = 1.0, double = 1.0);

	void RemoveSmallIsos(double);
	void ResampleIsos(double);
	void ResampleSplineIsos(double);
	void SmoothIsos(double, double, int = 1);
	void SmoothInsidePoint(const Vector2&);
	void DisplacementAlongCurve(const QVector<Vector2>&, double, double, double = 2);
	void Warp(const QVector<Vector2>&, double, double, double = 2, double = 1);

	void ViewPathFinished(const QVector<Vector2>&);
	void ViewRightClick(const Vector2&);

	// Protection zones
	void ProtectIsos(IsoLines&);
	bool WithinProtectZone(const Vector2&);
	void ProtectZone(const Vector2&);
	void ProtectSmallIsolines(double);
	void ProtectIsoline(int);
	void CleanProtectedZones();

	// Removing
	void RemoveIsolines(const QSet<int>&, bool = false);
	void RemoveSmallIsolines(double);
	void RemoveSelectedIsolines();

	// Undo/Redo isolines
	void SaveCurrentIsolines();
	void Undo();
	void Redo();

protected:
	Polygon2 PolygonReunion();
	Polygon2 PolygonHawaii();
	Polygon2 PolygonMan();

private:
	// UI & viewer
	Ui::Assets m_uiw;
	TerrainRaytracingWidget* m_meshWidget;

	// Current heightfield and isos
	HeightField m_hf;
	IsoLines m_isos;

	// Manage Undo/Redo process
	QVector<IsoLines> m_isos_historic = { IsoLines() };
	int m_current_historic_index = 0;

	// Avoid complex UI drawing while generating figures
	bool generate_figures = false;

	// Time taken for the generation displayed on UI
	qint64 m_test_ms_taken;

protected:
	// param for camera
	double m_camera_longitude;
	double m_camera_latitude;
	double m_camera_dist;
	Vector m_camera_lookat;

	// param for generation
	Box2 m_generation_box;
	HistogramD m_generation_histogram;
	ScalarField2 m_generation_noise;
	ScalarField2 m_generation_mask;
	double m_generation_radius;

	// results of generation
	GraphPoisson m_generation_graph_zones;
	GraphPoisson m_generation_graph_proba;
	GraphPoisson m_generation_graph_result;
	GraphPoisson m_generation_graph_heights;
	QVector<GraphPoisson> m_generation_graph_eden_asc;
	QVector<GraphPoisson> m_generation_graph_eden_desc;
	QVector<GraphPoisson> m_generation_graph_eden_final;

	// for ui and browser research
	QString m_histogram_directory = "../Data/dems/";
	QString m_generation_histogram_file;
	QString m_noise_directory = "../Images/input/";
	QString m_generation_noise_file;
	ScalarField2 m_generation_noise_without_pow;
	QString m_mask_directory = "../Data/dems/";
	QString m_generation_mask_file;

	// param for edition
	enum TerrainType{ STAIRS, INTERPOLATE, SMOOTH_STAIRS, ERODED, HEAT };
	TerrainType m_terrain_type;
	Box2 m_edition_box;
	double m_min_height;
	double m_max_height;
	int m_terrain_x;
	int m_terrain_y;

	enum EditTool { DRAWING, SMOOTHING, WARPING, SLOPING, PROTECTING };
	EditTool m_current_tool;
	double m_edition_radius;
	double m_edition_point_min_dist;

	double m_smoothing_delta;
	double m_smoothing_dist;

	double m_warping_pow;
	double m_warping_strength;
	double m_warping_factor;

	double m_sloping_pow;
	double m_sloping_strength;

	// param for extraction
	ExtractionOpt m_extraction_opt;

	// protected zones
	bool m_protecting_remove;
	double m_protecting_iso_max_length;
	QVector<Circle2> m_protected_zones;
};
