#ifndef PLAYLIST_WIDGET_HPP
#define PLAYLIST_WIDGET_HPP

#include <QWidget>

class QListWidget;
class QPushButton;
class QLineEdit;

class PlaylistWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlaylistWidget(
        QWidget *parent = nullptr
    );

private slots:

    void addFile();
    void removeSelected();
    void searchTracks();

private:

    QListWidget *playlistView;

    QLineEdit *searchBox;

    QPushButton *addButton;
    QPushButton *removeButton;
};

#endif