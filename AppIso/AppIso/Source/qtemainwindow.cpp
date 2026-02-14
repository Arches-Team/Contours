#include "qte.h"
#include <QtGui/QKeyEvent>


//static bool isClicking = false;

/*!
\brief Initialize the window and the callbacks.
*/
MainAmplificationWindow::MainAmplificationWindow()
{
	// Chargement de l'interface
	m_uiw.setupUi(this);
	m_uiw.tabWidget->setCurrentIndex(0);

	// Chargement du GLWidget
	m_meshWidget = new TerrainRaytracingWidget();

	QGridLayout* GLlayout2 = new QGridLayout;
	GLlayout2->addWidget(m_meshWidget, 0, 0);
	GLlayout2->setContentsMargins(0, 0, 0, 0);
	m_uiw.widget_GL2->setLayout(GLlayout2);

	// Camera
	//m_meshWidget->SetNearAndFarPlane(Vector2(1.0, 50000));
	m_meshWidget->SetCamera(Camera::View(Box(Vector(0), 2500)));

	// Met le splitter central au bon endroit
	m_uiw.splitter->setSizes({ 200, 50 });
	m_uiw.splitter->setCollapsible(0, false);

	// Callbacks interface
	CreateActions();
}

/*!
\brief Destructor
*/
MainAmplificationWindow::~MainAmplificationWindow()
{
	delete m_meshWidget;
}

void MainAmplificationWindow::keyPressEvent(QKeyEvent* event)
{
	if (event->matches(QKeySequence::Undo))
	{
		Undo();
	}
	else if (event->matches(QKeySequence::Redo))
	{
		Redo();
	}
	else
	{
		QWidget::keyPressEvent(event);
	}
}

/*!
\brief Creates callbacks interface
*/
void MainAmplificationWindow::CreateActions()
{
	// Misc
	connect(m_uiw.resetCamera,         SIGNAL(clicked()), this, SLOT(DefaultCamera()));
	connect(m_uiw.lookdownCamera,      SIGNAL(clicked()), this, SLOT(LookDownCamera()));
	connect(m_uiw.smooth_button,        SIGNAL(clicked()), this, SLOT(SmoothHeightField()));
	connect(m_uiw.scale_button, SIGNAL(clicked()), this, SLOT(ScaleButton()));

	//////////////////////////
	///   ISOLINES PAPER   ///
	//////////////////////////

	connect(m_uiw.generate_figures_button, &QPushButton::clicked, this, &MainAmplificationWindow::GenerateFigures);

	// Export Images
	connect(m_uiw.export_scene_button, &QPushButton::clicked, this, [this]() { ExportSceneImage(); });
	connect(m_uiw.export_stairs_button, &QPushButton::clicked, this, [this]() { ExportIsosStairs("", m_terrain_x, m_terrain_y); });
	connect(m_uiw.export_smooth_stairs_button, &QPushButton::clicked, this, [this]() { ExportIsosSmoothStairs("", m_terrain_x, m_terrain_y); });
	connect(m_uiw.export_interpolate_button, &QPushButton::clicked, this, [this]() { ExportIsosInterpolate("", m_terrain_x, m_terrain_y); });
	connect(m_uiw.export_isolines_button, &QPushButton::clicked, this, [this]() { ExportIsosSVG(); });
	connect(m_uiw.export_all_button, &QPushButton::clicked, this, [this]() { ExportAll(m_uiw.export_path_edit->text()); });
	connect(this, &MainAmplificationWindow::isoChanged, [this](IsoLines* isos) {
		bool enable = isos != nullptr && isos->Size() != 0;
		m_uiw.export_stairs_button->setEnabled(enable);
		m_uiw.export_smooth_stairs_button->setEnabled(enable);
		m_uiw.export_interpolate_button->setEnabled(enable);
		m_uiw.export_pbr_button->setEnabled(enable);
		m_uiw.export_isolines_button->setEnabled(enable);
		m_uiw.export_all_button->setEnabled(enable);
	});
	connect(m_uiw.export_pbr_button, &QPushButton::clicked, this, [this]() { ExportMSEAndPBRRender(); });

	ResetExportOptions();

	// Génération
	connect(m_uiw.generation_box_y_edit, &QLineEdit::editingFinished, this, &MainAmplificationWindow::ResetGenBox);
	connect(m_uiw.generation_box_h_edit, &QLineEdit::editingFinished, this, &MainAmplificationWindow::ResetGenBox);
	connect(m_uiw.generation_box_w_edit, &QLineEdit::editingFinished, this, &MainAmplificationWindow::ResetGenBox);
	connect(m_uiw.generation_box_x_edit, &QLineEdit::editingFinished, this, &MainAmplificationWindow::ResetGenBox);

	connect(m_uiw.histogram_nb_isos_edit, &QLineEdit::editingFinished, this, [this]() { ChangeHistogramFromUI(); });
	connect(m_uiw.noise_pow_edit, &QLineEdit::editingFinished, this, [this]() { ChangeNoisePower(m_uiw.noise_pow_edit->text().toDouble()); });
	connect(m_uiw.mask_nb_isos_edit, &QLineEdit::editingFinished, [this]() { ChangeMaskParams(m_uiw.mask_nb_isos_edit->text().toInt(), m_uiw.sea_level_edit->text().toDouble()); });
	connect(m_uiw.sea_level_edit, &QLineEdit::editingFinished, [this]() { ChangeMaskParams(m_uiw.mask_nb_isos_edit->text().toInt(), m_uiw.sea_level_edit->text().toDouble()); });
	connect(m_uiw.poisson_radius_edit, &QLineEdit::editingFinished, this, &MainAmplificationWindow::ResetPoissonRadius);

	connect(m_uiw.histogram_button, &QPushButton::clicked, this, &MainAmplificationWindow::BrowseHistogram);
	connect(m_uiw.histogram_function_selector, QOverload<int>::of(&QComboBox::activated), this, [this](int) { ChangeHistogramFromUI(); });
	connect(m_uiw.noise_button, &QPushButton::clicked, this, &MainAmplificationWindow::BrowseNoise);
	connect(m_uiw.mask_button, &QPushButton::clicked, this, &MainAmplificationWindow::BrowseMask);
	connect(m_uiw.mask_from_view_isos_button, &QPushButton::clicked, this, &MainAmplificationWindow::ChangeMaskFromCurrentIsos);

	connect(m_uiw.generation_button, &QPushButton::clicked, this, [this]() { Generate(3); });

	ResetGenBox();
	ChangeHistogramFromFile(m_histogram_directory + "reunion.png", 12);
	ChangeMaskFromFile(m_mask_directory + "reunion.png");
	ChangeNoiseFromFile(m_noise_directory + "uniform.png");

	// Édition
	connect(m_uiw.view_box_y_edit, &QLineEdit::editingFinished, this, &MainAmplificationWindow::ResetViewBox);
	connect(m_uiw.view_box_h_edit, &QLineEdit::editingFinished, this, &MainAmplificationWindow::ResetViewBox);
	connect(m_uiw.view_box_w_edit, &QLineEdit::editingFinished, this, &MainAmplificationWindow::ResetViewBox);
	connect(m_uiw.view_box_x_edit, &QLineEdit::editingFinished, this, &MainAmplificationWindow::ResetViewBox);

	connect(m_uiw.heights_max_edit, &QLineEdit::editingFinished, this, &MainAmplificationWindow::ResetHeights);
	connect(m_uiw.heights_min_edit, &QLineEdit::editingFinished, this, &MainAmplificationWindow::ResetHeights);

	connect(m_uiw.terrain_x_edit, &QLineEdit::editingFinished, this, &MainAmplificationWindow::ResetTerrainSize);
	connect(m_uiw.terrain_y_edit, &QLineEdit::editingFinished, this, &MainAmplificationWindow::ResetTerrainSize);

	connect(m_uiw.camera_longitude_angle_edit, &QLineEdit::editingFinished, this, &MainAmplificationWindow::ResetCamera);
	connect(m_uiw.camera_latitude_angle_edit, &QLineEdit::editingFinished, this, &MainAmplificationWindow::ResetCamera);
	connect(m_uiw.camera_dist_edit, &QLineEdit::editingFinished, this, &MainAmplificationWindow::ResetCamera);
	connect(m_uiw.camera_lookat_x_edit, &QLineEdit::editingFinished, this, &MainAmplificationWindow::ResetCamera);
	connect(m_uiw.camera_lookat_y_edit, &QLineEdit::editingFinished, this, &MainAmplificationWindow::ResetCamera);
	connect(m_uiw.camera_lookat_z_edit, &QLineEdit::editingFinished, this, &MainAmplificationWindow::ResetCamera);
	connect(m_meshWidget, &MeshWidget::_signalMouseMove, [this]() { ChangeCamera(m_meshWidget->GetCamera()); });
	
	m_uiw.terrain_type_group->setId(m_uiw.stairs_radio, STAIRS);
	m_uiw.terrain_type_group->setId(m_uiw.interpolate_radio, INTERPOLATE);
	m_uiw.terrain_type_group->setId(m_uiw.smooth_stairs_radio, SMOOTH_STAIRS);
	m_uiw.terrain_type_group->setId(m_uiw.heat_radio, ERODED);
	m_uiw.stairs_radio->setChecked(true);
	connect(m_uiw.terrain_type_group, &QButtonGroup::idToggled, this, [this](int id, bool checked) { if (checked) { ResetTerrainSize(); } });

	connect(m_uiw.edition_tool_selector, &QComboBox::currentIndexChanged, this, &MainAmplificationWindow::ChangeEditionTool);

	connect(m_uiw.edition_radius_edit, &QLineEdit::textChanged, this, &MainAmplificationWindow::ResetEditionOptions);
	connect(m_uiw.point_min_dist_edit, &QLineEdit::textChanged, this, &MainAmplificationWindow::ResetEditionOptions);
	// Pour avoir la meme chose entre le raidus de l'iso et celui de l'UI
	connect(m_uiw.iso_view, &IsoView::radiusChanged, [this](double r) { m_uiw.edition_radius_edit->setText(QString::number(r)); ResetEditionOptions(); });

	connect(m_uiw.edition_delta_edit, &QDoubleSpinBox::textChanged, this, &MainAmplificationWindow::ResetSmoothingOptions);
	connect(m_uiw.edition_dist_edit, &QLineEdit::textChanged, this, &MainAmplificationWindow::ResetSmoothingOptions);

	connect(m_uiw.warp_strength_edit, &QDoubleSpinBox::textChanged, this, &MainAmplificationWindow::ResetWarpingOptions);
	connect(m_uiw.warp_factor_eps_edit, &QDoubleSpinBox::textChanged, this, &MainAmplificationWindow::ResetWarpingOptions);
	connect(m_uiw.warp_pow_edit, &QDoubleSpinBox::textChanged, this, &MainAmplificationWindow::ResetWarpingOptions);

	connect(m_uiw.slope_pow_edit, &QDoubleSpinBox::textChanged, this, &MainAmplificationWindow::ResetSlopingOptions);
	connect(m_uiw.slope_strength_edit, &QDoubleSpinBox::textChanged, this, &MainAmplificationWindow::ResetSlopingOptions);

	connect(m_uiw.toggle_remove_circles, &QCheckBox::toggled, this, &MainAmplificationWindow::ResetProtectingOptions);
	connect(m_uiw.toggle_show_zones, &QCheckBox::toggled, this, &MainAmplificationWindow::ResetProtectingOptions);
	connect(m_uiw.protect_small_isolines_edit, &QLineEdit::textChanged, this, &MainAmplificationWindow::ResetProtectingOptions);
	connect(m_uiw.clean_protected_zones_button, &QPushButton::clicked, this, &MainAmplificationWindow::CleanProtectedZones);
	connect(m_uiw.protect_small_isolinesbutton, &QPushButton::clicked, this, [this]() { ProtectSmallIsolines(m_protecting_iso_max_length); });

	connect(m_uiw.clean_view_button, &QPushButton::clicked, this, &MainAmplificationWindow::CleanIsos);
	connect(m_uiw.reset_zoom_button, &QPushButton::clicked, this, &MainAmplificationWindow::ResetViewBox);
	connect(m_uiw.reset_from_mask_button, &QPushButton::clicked, this, &MainAmplificationWindow::SetIsosFromMask);

	connect(m_uiw.iso_view, &IsoView::pathFinished, this, &MainAmplificationWindow::ViewPathFinished);
	connect(m_uiw.iso_view, &IsoView::rightClickOn, this, &MainAmplificationWindow::ViewRightClick);
	connect(m_uiw.iso_view, &IsoView::keyPressed, this, [this](int key) { // create every keypress that needs to be handle only in the view
		if (key == Qt::Key_Delete)
			RemoveSelectedIsolines();
	});

	connect(m_uiw.mse_button, &QPushButton::clicked, this, [this]() { MSE(); });
	connect(m_uiw.resample_isos_button, &QPushButton::clicked, this, [this]() { ResampleIsos(m_edition_point_min_dist); });
	connect(m_uiw.resample_spline_isos_button, &QPushButton::clicked, this, [this]() { ResampleSplineIsos(m_edition_point_min_dist); });

	ResetViewBox();
	ResetHeights();
	ResetTerrainSize();
	DefaultCamera();
	ResetEditionOptions();
	ResetSmoothingOptions();
	ResetWarpingOptions();
	ResetSlopingOptions();
	ResetProtectingOptions();
	ChangeEditionTool(DRAWING);

	// Helper
	connect(m_uiw.load_reunion_poly_button, &QPushButton::clicked, this, [&]() { ResetViewWithPolygon(PolygonReunion()); });
	connect(m_uiw.load_man_poly_button, &QPushButton::clicked, this, [&]() { ResetViewWithPolygon(PolygonMan()); });
	connect(m_uiw.load_hawaii_poly_button, &QPushButton::clicked, this, [&]() { ResetViewWithPolygon(PolygonHawaii()); });

	SetIsos({});
}
