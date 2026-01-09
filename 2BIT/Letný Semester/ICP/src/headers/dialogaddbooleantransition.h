/**
 * @file dialogaddbooleantransition.h
 * @author Eliška Křeménková (xkremee00)
 * @brief Declaration of the DialogAddBooleanTransition class that handles adding new boolean transition to the model
 * @date 2025-05-09
 */

#ifndef DIALOGADDBOOLEANTRANSITION_H
#define DIALOGADDBOOLEANTRANSITION_H

#include <QDialog>

namespace Ui {
class DialogAddBooleanTransition;
}

/**
 * @class DialogAddBooleanTransition
 * @brief A dialog window for creating and configuring a new boolean transition.
 *
 * This class provides a simple UI for entering
 * - source and target state name
 * - boolean condition
 * 
 * It is used as a modal dialog and returns user input
 * through getter methods after acceptance.
 */
class DialogAddBooleanTransition : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Construct a new Dialog Add Boolean Transition object
     * @param parent Parent widget, default is nullptr
     */
    explicit DialogAddBooleanTransition(QWidget *parent = 0);

    /**
     * @brief Destroy the Dialog Add Boolean Transition object
     */
    ~DialogAddBooleanTransition();

    /**
     * @brief Set available states to the dialog's combo boxes
     * @param stateNames List of available state names
     */
    void setAvailableStates(const QStringList& stateNames);

    /**
     * @brief Set available variables to the dialog's list widget
     * 
     * show machine variables to choose for the condition
     * 
     * @param varNames List of available variables
     */
    void setAvailableVars(const QStringList& varNames);

    /**
     * @brief Get the transition source state name from the dialog
     * @return name
     */
    QString getFromState() const;

    /**
     * @brief Get the transition target state name from the dialog
     * @return name
     */
    QString getToState() const;

    /**
     * @brief Get the boolean condition's left operand from the dialog
     * @return variable name or operand's value as string
     */
    QString getLeftOp() const;

    /**
     * @brief Get the boolean condition's operation from the dialog
     * @return variable name or operand's value as string
     */
    QString getOp() const;

    /**
     * @brief Get the boolean condition's right operand from the dialog
     * @return variable name or operand's value as string
     */
    QString getRightOp() const;

private:
    Ui::DialogAddBooleanTransition *ui;
};

#endif // DIALOGADDBOOLEANTRANSITION_H
