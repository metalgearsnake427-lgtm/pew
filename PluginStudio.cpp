#include "PluginStudio.hpp"

#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>

PluginStudio::PluginStudio(QWidget *parent)
    : QWidget(parent)
{
    pluginList = new QListWidget;

    installButton = new QPushButton("Install");
    removeButton = new QPushButton("Remove");
    enableButton = new QPushButton("Enable");
    disableButton = new QPushButton("Disable");
    createButton = new QPushButton("Create Plugin");

    pluginName = new QLabel("Name: None");
    pluginVersion = new QLabel("Version: Unknown");
    pluginAuthor = new QLabel("Author: Unknown");

    pluginDescription = new QTextEdit;
    pluginDescription->setReadOnly(true);

    auto *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(pluginList);

    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(installButton);
    buttonLayout->addWidget(removeButton);

    leftLayout->addLayout(buttonLayout);

    auto *rightLayout = new QVBoxLayout;

    rightLayout->addWidget(pluginName);
    rightLayout->addWidget(pluginVersion);
    rightLayout->addWidget(pluginAuthor);
    rightLayout->addWidget(pluginDescription);

    auto *controlLayout = new QHBoxLayout;
    controlLayout->addWidget(enableButton);
    controlLayout->addWidget(disableButton);
    controlLayout->addWidget(createButton);

    rightLayout->addLayout(controlLayout);

    auto *mainLayout = new QHBoxLayout;

    mainLayout->addLayout(leftLayout, 1);
    mainLayout->addLayout(rightLayout, 2);

    setLayout(mainLayout);

    setWindowTitle("VIDM Plugin Studio");

    connect(
        installButton,
        &QPushButton::clicked,
        this,
        &PluginStudio::installPlugin
    );

    connect(
        removeButton,
        &QPushButton::clicked,
        this,
        &PluginStudio::removePlugin
    );

    connect(
        enableButton,
        &QPushButton::clicked,
        this,
        &PluginStudio::enablePlugin
    );

    connect(
        disableButton,
        &QPushButton::clicked,
        this,
        &PluginStudio::disablePlugin
    );

    connect(
        createButton,
        &QPushButton::clicked,
        this,
        &PluginStudio::createPlugin
    );
}

void PluginStudio::installPlugin()
{
}

void PluginStudio::removePlugin()
{
}

void PluginStudio::enablePlugin()
{
}

void PluginStudio::disablePlugin()
{
}

void PluginStudio::createPlugin()
{
}