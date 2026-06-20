#ifndef VIDM_MAINWINDOW_HPP
#define VIDM_MAINWINDOW_HPP

#include <QMainWindow>

class QPushButton;
class QSlider;
class QLabel;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onPlayPressed();
    void onPausePressed();
    void onOpenPressed();

private:
    QPushButton* m_playButton;
    QPushButton* m_pauseButton;
    QPushButton* m_openButton;

    QSlider* m_timeline;
    QSlider* m_volume;

    QLabel* m_status;
};

#endif