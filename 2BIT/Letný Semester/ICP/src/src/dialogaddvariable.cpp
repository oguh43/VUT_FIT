/**
 * @file dialogaddvariable.h
 * @author Eliška Křeménková (xkremee00)
 * @brief Implementation of the DialogAddVariable class that handles adding new variable to the model
 * @date 2025-05-09
 */

#include "../headers/dialogaddvariable.h"
#include "ui_dialogaddvariable.h"

/**
 * @brief Construct a new DialogAddVariable object
 * 
 * @param parent Parent widget, default is nullptr.
 */
DialogAddVariable::DialogAddVariable(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAddVariable)
{
    ui->setupUi(this);
}

/**
 * @brief Destroy the DialogAddVariable object
 */
DialogAddVariable::~DialogAddVariable()
{
    delete ui;
}

/**
 * @brief Get the Var Name object
 * 
 * @return QString name of the variable received from the dialog
 */
QString DialogAddVariable::getVarName() const {
    return ui->lineEdit_add_var_name->text();
}

/**
 * @brief Get the Var Type object
 * 
 * @return QString type of the variable received from the dialog (available types: string, int, float)
 */
QString DialogAddVariable::getVarType() const {
    return ui->comboBox_add_var_type->currentText();
}

/**
 * @brief Get the Var Value object
 * 
 * @return QString value of the variable received from the dialog
 */
QString DialogAddVariable::getVarValue() const {
    return ui->lineEdit_add_var_value->text();
}
