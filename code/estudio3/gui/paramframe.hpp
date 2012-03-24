/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   This file is part of
 *       _______   ______________  ______     _____
 *      / ____/ | / /  _/ ____/  |/  /   |   |__  /
 *     / __/ /  |/ // // / __/ /|_/ / /| |    /_ <
 *    / /___/ /|  // // /_/ / /  / / ___ |  ___/ /
 *   /_____/_/ |_/___/\____/_/  /_/_/  |_| /____/.
 *
 *   Copyright © 2003-2010 Brain Control, all rights reserved.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef PARAM_FRAME_HPP
#define PARAM_FRAME_HPP

#include <QtGui/QScrollArea>

#include "../../eshared/eshared.hpp"

class QVBoxLayout;

// Frame which displays all parameters of an
// operator. The user can edit the parameters.
// If a parameter has changed, the operator-
// changed signal is emitted.
class eParameterFrame : public QScrollArea
{
    Q_OBJECT

public:
    eParameterFrame(QWidget *parent=eNULL);
    virtual ~eParameterFrame();

    void            setOperator(eID opId);
    eID             getOperator() const;

Q_SIGNALS:
    void            onOperatorChanged(const eParameter *param);
    void            onGotoOperator(eID opId);

private:
    void            _createWidgets();
    void            _clearWidgets();
    void            _clearLayout(QLayout *layout);

    void            _addMainSeparator();
    void            _addDefaultParams(const eIOperator *op);
    void            _addSeparatorLabel(const QString &caption, eU32 row);

private Q_SLOTS:
    void            _onBypassClicked(bool checked);
    void            _onHideClicked(bool checked);
    void            _onNameChanged(const QString &text);
    void            _onParameterChanged(const eParameter &param);
	void			_onDefaultNameClicked();
private:
    eID             m_opId;
    QVBoxLayout *   m_layout;
	void*			m_nameField;
};

#endif // PARAM_FRAME_HPP