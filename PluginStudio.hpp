#ifndef VIDM_PLUGIN_STUDIO_HPP
#define VIDM_PLUGIN_STUDIO_HPP

#include <QWidget>

class QListWidget;
class QPushButton;
class QLabel;
class QTextEdit;

class PluginStudio : public QWidget
{
    Q_OBJECT

public:
    explicit PluginStudio(QWidget *parent = nullptr);

private slots:
    void installPlugin();
    void removePlugin();
    void enablePlugin();
    void disablePlugin();
    void createPlugin();

private:
    QListWidget *pluginList;

    QPushButton *installButton;
    QPushButton *removeButton;
    QPushButton *enableButton;
    QPushButton *disableButton;
    QPushButton *createButton;

    QLabel *pluginName;
    QLabel *pluginVersion;
    QLabel *pluginAuthor;

    QTextEdit *pluginDescription;
};

#endif