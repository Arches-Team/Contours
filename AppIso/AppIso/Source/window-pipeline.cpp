#include "qte.h"

void MainAmplificationWindow::UpdateGeometry(bool entire) {
    if (entire) {
        m_meshWidget->SetHeightField(&m_hf);
    }
    else {
        m_meshWidget->UpdateInternal();
    }
}

void MainAmplificationWindow::ScaleButton() {
    double scaling_factor = m_uiw.scale_input->value();
    m_hf.Scale(Vector(1, 1, scaling_factor));
    UpdateGeometry(false);
}

void MainAmplificationWindow::SmoothHeightField()
{
    m_hf.Smooth();
    UpdateGeometry(false);
}