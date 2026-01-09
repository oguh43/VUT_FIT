/**
 * @file dialogedittransition.cpp
 * @author Eliška Křeménková (xkremee00)
 * @brief Implementation of the DialogEditTransition class that handles editing transition timeout and input conditions
 * @date 2025-05-10
 */

#include "../headers/dialogedittransition.h"
#include "ui_dialogedittransition.h"

/**
 * @brief Construct a new Dialog Edit Transition object
 * @param parent Parent widget, default is nullptr
 */
DialogEditTransition::DialogEditTransition(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEditTransition)
{
    ui->setupUi(this);
}

/**
 * @brief Destroy the Dialog Edit Transition object
 */
DialogEditTransition::~DialogEditTransition()
{
    delete ui;
}

/**
 * @brief Add a new input condition to the list of input conditions inside the dialog
 *
 * Triggered when the user clicks the "Add" button
 * and adds data from input fields (e.g., input symbol and pointer) to the list
 */
void DialogEditTransition::on_pushButton_add_condition_clicked()
{
    QString symbol = ui->lineEdit_input_symbol->text().trimmed();
    QString pointer = ui->comboBox_input_pointer->currentText().trimmed();

    if (symbol.isEmpty() || pointer.isEmpty())
        return;

    QString condition = QString("%1 from %2").arg(symbol, pointer);
    ui->listWidget_add_conditions->addItem(condition);

    ui->lineEdit_input_symbol->clear();
}

/**
 * @brief Remove selected input condition from the list of input conditions inside the dialog
 *
 * Triggered when the user clicks the "Remove" button
 * and removes selected condition from the list
 */
void DialogEditTransition::on_pushButton_remove_condition_clicked()
{
    QListWidgetItem* item = ui->listWidget_add_conditions->currentItem();
    if (item) {
        delete item;
    }
}

/**
 * @brief Set available input pointers to the dialog's combo box
 * @param pointers List of available pointers
 */
void DialogEditTransition::setInputPointers(const QStringList& pointers) {
    ui->comboBox_input_pointer->clear();
    ui->comboBox_input_pointer->addItems(pointers);
}

/**
 * @brief Get the input conditions from the dialog
 * @return list of conditions stored as string pairs
 */
QList<QPair<QString, QString>> DialogEditTransition::getInputConditions() const {
    QList<QPair<QString, QString>> conditions;

    for (int i = 0; i < ui->listWidget_add_conditions->count(); ++i) {
        QString text = ui->listWidget_add_conditions->item(i)->text();
        QStringList parts = text.split(" from ");
        if (parts.size() == 2) {
            conditions.append(qMakePair(parts[0], parts[1]));
        }
    }

    return conditions;
}

/**
 * @brief Get the timeout for of the transition from the dialog
 * @return timeout in milliseconds
 */
int DialogEditTransition::getTimeout() const {
    return ui->spinBox_timeout->value();
}

/**
 * @brief Set available input condition
 * @param conditions List of available input conditions
 */
void DialogEditTransition::setInputConditions(const QList<QPair<QString, QString>> &conditions)
{
    ui->listWidget_add_conditions->clear();

    for (const QPair<QString, QString> &cond : conditions) {
        QString text = QString("%1 from %2").arg(cond.first, cond.second);
        ui->listWidget_add_conditions->addItem(text);
    }
}

/**
 * @brief Set the timeout of the transition to the dialog's spin box
 * @param timeout Timeout in milliseconds
 */
void DialogEditTransition::setTimeout(int timeout) {
    ui->spinBox_timeout->setValue(timeout);
}
