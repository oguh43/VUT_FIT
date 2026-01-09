/**
 * @file dialogaddtransition.cpp
 * @author Eliška Křeménková (xkremee00)
 * @brief Implementation of the DialogAddTransition class that handles adding new transition to the model
 * @date 2025-05-09
 */

#include "../headers/dialogaddtransition.h"
#include "ui_dialogaddtransition.h"

/**
 * @brief Construct a new Dialog Add Transition object
 * 
 * @param parent Parent widget, default is nullptr
 */
DialogAddTransition::DialogAddTransition(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAddTransition)
{
    ui->setupUi(this);
}

/**
 * @brief Destroy the Dialog Add Transition object
 */
DialogAddTransition::~DialogAddTransition()
{
    delete ui;
}

/**
 * @brief Set available states to the dialog's combo boxes
 * @param stateNames List of available state names
 */
void DialogAddTransition::setAvailableStates(const QStringList &stateNames)
{
    ui->comboBox_from->clear();
    ui->comboBox_to->clear();

    ui->comboBox_from->addItems(stateNames);
    ui->comboBox_to->addItems(stateNames);
}

/**
 * @brief Set available states to the dialog's combo boxes
 * @param pointers List of available state names
 */
void DialogAddTransition::setInputPointers(const QStringList& pointers) {
    ui->comboBox_input_pointer->clear();
    ui->comboBox_input_pointer->addItems(pointers);
}

/**
 * @brief Get the transition source state name from the dialog
 * @return name
 */
QString DialogAddTransition::getFromState() const {
    return ui->comboBox_from->currentText();
}

/**
 * @brief Get the transition target state name from the dialog
 * @return name
 */
QString DialogAddTransition::getToState() const {
    return ui->comboBox_to->currentText();
}

/**
 * @brief Get the input conditions from the dialog
 * @return list of conditions stored as string pairs
 */
QList<QPair<QString, QString>> DialogAddTransition::getInputConditions() const {
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
int DialogAddTransition::getTimeout() const {
    return ui->spinBox_timeout->value();
}

/**
 * @brief Add a new input condition to the list of input conditions inside the dialog
 * 
 * Triggered when the user clicks the "Add" button
 * and adds data from input fields (e.g., input symbol and pointer) to the list
 */
void DialogAddTransition::on_pushButton_add_condition_clicked()
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
void DialogAddTransition::on_pushButton_remove_condition_clicked()
{
    QListWidgetItem* item = ui->listWidget_add_conditions->currentItem();
    if (item) {
        delete item;
    }
}
