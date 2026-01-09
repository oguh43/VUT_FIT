/**
 * @file dialogaddstate.cpp
 * @author Eliška Křeménková (xkremee00)
 * @brief Implementation of the DialogAddState class that handles adding new state to the model
 * @date 2025-05-09
 */

#include "../headers/dialogaddstate.h"
#include "ui_dialogaddstate.h"

/**
 * @brief Construct a new Dialog Add State object
 * @param parent Parent widget, default is nullptr
 */
DialogAddState::DialogAddState(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAddState)
{
    ui->setupUi(this);
}

/**
 * @brief Destroy the Dialog Add State object
 */
DialogAddState::~DialogAddState()
{
    delete ui;
}

/**
 * @brief Get the state name from the dialog
 * @return name
 */
QString DialogAddState::getStateName() const {
    return ui->lineEdit_add_state_name->text().trimmed();
}

/**
 * @brief Check if the state was set as initial in the dialog
 * @return true if set as Initial
 */
bool DialogAddState::isStartState() const {
    return ui->checkBox_add_state_start->isChecked();
}

/**
 * @brief Check if the state was set as final in the dialog
 * @return true if set as Final
 */
bool DialogAddState::isFinalState() const {
    return ui->checkBox_add_state_final->isChecked();
}

/**
 * @brief Get the state x position from the dialog
 * @return x position
 */
int DialogAddState::getPosX() const {
    return ui->spinBox_add_state_x->value();
}

/**
 * @brief Get the state y position from the dialog
 * @return y position
 */
int DialogAddState::getPosY() const {
    return ui->spinBox_add_state_y->value();
}
