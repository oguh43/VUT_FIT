/**
 * @file mainwindow.cpp
 * @author Eliška Křeménková (xkremee00)
 * @brief main window of the application
 * @date 2025-05-09
 */

#include "../headers/mainwindow.h"
#include "ui_mainwindow.h"
#include <QString>
#include <QMessageBox>
#include <QFileDialog>

/**
 * @brief Construct a new Main Window object
 * @param parent Parent QWidget, used for memory management
 */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("FSM Editor");
}

/**
 * @brief Destroy the Main Window object
 */
MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief Slot for handling "Create New" button click
 * 
 * This will open the automaton editor window with clean graphic scene
 */
void MainWindow::on_pushButton_create_new_clicked()
{
    QString name = ui->lineEdit_new_name->text().trimmed();
    if (name.isEmpty()){
        QMessageBox::warning(this, "Missing Name", "Please enter a name for the automaton.");
        return;
    }

    auto *editor = new AutomatonEditor(name);
    editor->show();
    this->hide();
}

/**
 * @brief Slot for handling "Open from" button click
 * 
 * This will open the automaton editor window with loaded automaton from the given file
 */
void MainWindow::on_pushButton_open_from_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open Automaton",
                                                   QString(), "Automaton Files (*.fsm);;All Files (*)");
    if (filePath.isEmpty()) {
        return;
    }

    // Create an editor and load the file
    auto *editor = new AutomatonEditor(filePath, true);
    editor->show();
    this->hide();
}
