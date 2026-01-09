/**
 * @file dialogaddbooleantransition.cpp
 * @author Eliška Křeménková (xkremee00)
 * @brief Implementation of the DialogAddBooleanTransition class that handles adding new boolean transition to the model
 * @date 2025-05-09
 */

#include "../headers/dialogaddbooleantransition.h"
#include "ui_dialogaddbooleantransition.h"

/**
 * @brief Construct a new Dialog Add Boolean Transition object
 * @param parent Parent widget, default is nullptr
 */
DialogAddBooleanTransition::DialogAddBooleanTransition(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAddBooleanTransition)
{
    ui->setupUi(this);
}

/**
 * @brief Destroy the Dialog Add Boolean Transition object
 */
DialogAddBooleanTransition::~DialogAddBooleanTransition()
{
    delete ui;
}

/**
 * @brief Set available states to the dialog's combo boxes
 * @param stateNames List of available state names
 */
void DialogAddBooleanTransition::setAvailableStates(const QStringList &stateNames) {
    ui->comboBox_from->clear();
    ui->comboBox_to->clear();

    ui->comboBox_from->addItems(stateNames);
    ui->comboBox_to->addItems(stateNames);
}

/**
 * @brief Set available variables to the dialog's list widget
 * 
 * show machine variables to choose for the condition
 * 
 * @param varNames List of available variables
 */
void DialogAddBooleanTransition::setAvailableVars(const QStringList &varNames) {
    ui->listWidget_choose_variable->addItems(varNames);
}

/**
 * @brief Get the transition source state name from the dialog
 * @return name
 */
QString DialogAddBooleanTransition::getFromState() const {
    return ui->comboBox_from->currentText();
}

/**
 * @brief Get the transition target state name from the dialog
 * @return name
 */
QString DialogAddBooleanTransition::getToState() const {
    return ui->comboBox_to->currentText();
}

/**
 * @brief Get the boolean condition's left operand from the dialog
 * @return variable name or operand's value as string
 */
QString DialogAddBooleanTransition::getLeftOp() const {
    return ui->lineEdit_add_left_op->text();
}

/**
 * @brief Get the boolean condition's operation from the dialog
 * @return variable name or operand's value as string
 */
QString DialogAddBooleanTransition::getOp() const {
    return ui->comboBox_add_op->currentText();
}

/**
 * @brief Get the boolean condition's right operand from the dialog
 * @return variable name or operand's value as string
 */
QString DialogAddBooleanTransition::getRightOp() const {
    return ui->lineEdit_add_right_op->text();
}
