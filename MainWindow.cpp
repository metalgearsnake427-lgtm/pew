void MainWindow::adaptToDevice()
{
    QScreen* screen = QApplication::primaryScreen();

    QSize size = screen->availableGeometry().size();

    if(size.width() < 800)
    {
        // Phone
        m_isMobile = true;
    }
    else if(size.width() < 1280)
    {
        // Tablet
        m_isTablet = true;
    }
    else
    {
        // Desktop
        m_isDesktop = true;
    }
}
if(m_isMobile)
{
    playlistPanel->hide();

    controlsLayout->setDirection(
        QBoxLayout::TopToBottom
    );
}
else
{
    playlistPanel->show();

    controlsLayout->setDirection(
        QBoxLayout::LeftToRight
    );
}