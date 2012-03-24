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

#ifndef PARAM_WIDGETS_HPP
#define PARAM_WIDGETS_HPP

#include <QtGui/QTextEdit>
#include <QtGui/QLineEdit>
#include <QtGui/QComboBox>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSyntaxHighlighter>

#include "../../eshared/eshared.hpp"

// Line edit widget for numbers (integer and float).
// Value can be changed using mouse wheel or by
// dragging mouse horizontally.
class eTrackEdit : public QLineEdit
{
    Q_OBJECT

public:
    eTrackEdit(eParameter &param, eU32 component, QWidget *parent=eNULL);
    virtual ~eTrackEdit();

Q_SIGNALS:
    void            onParameterChanged(const eParameter &param);

private Q_SLOTS:
    void            _onTextEdited(const QString &text);

private:
    eBool           _changeValue(eF32 factor);
    eInt            _getInt() const;
    eF32            _getFloat() const;
    void            _setInt(eInt value);
    void            _setFloat(eF32 value);

private:
    virtual void    wheelEvent(QWheelEvent *we);
    virtual void    keyPressEvent(QKeyEvent *ke);
    virtual void    keyReleaseEvent(QKeyEvent *ke);
    virtual void    mouseMoveEvent(QMouseEvent *me);
    virtual void    mousePressEvent(QMouseEvent *me);
    virtual void    mouseReleaseEvent(QMouseEvent *me);
    virtual void    timerEvent(QTimerEvent *te);
    virtual void    focusInEvent(QFocusEvent *fe);
    virtual void    focusOutEvent(QFocusEvent *fe);

private:
    eParameter &    m_param;
    eU32            m_component;

    eBool           m_ctrlDown;
    eBool           m_shiftDown;
    eBool           m_valueChanged;
    QPoint          m_mouseDownPos;
    eInt            m_timerId;
};

// Implements a normal push button which changes
// its caption between true and false (yes and no),
// each time it's clicked.
class eBoolButton : public QPushButton
{
    Q_OBJECT

public:
    eBoolButton(eParameter &param, QWidget *parent=eNULL);

Q_SIGNALS:
    void            onParameterChanged(const eParameter &param);

private:
    void            _updateCaption();

private Q_SLOTS:
    void            _onClicked();

private:
    eParameter &    m_param;
};

// Writes index of selected combobox entry
// to parameter passed in constructor.
class eComboBox : public QComboBox
{
    Q_OBJECT

public:
    eComboBox(eParameter &param, QWidget *parent=eNULL);

Q_SIGNALS:
    void            onParameterChanged(const eParameter &param);

private Q_SLOTS:
    void            _onActivated(int index);

private:
    eParameter &    m_param;
};

// Flag button (state: down, not down). Writes
// its state to the parameter passed in the
// constructor.
class eFlagButton : public QPushButton
{
    Q_OBJECT

public:
    eFlagButton(eParameter &param, eU32 flagIndex, QWidget *parent=eNULL);

Q_SIGNALS:
    void            onParameterChanged(const eParameter &param);

private:
    void            _updateDownState();

private Q_SLOTS:
    void            _onClicked();

private:
    eParameter &    m_param;
    eU32            m_flagIndex;
};

// Multi-line edit field, which writes its
// text to parameter passed in constructor.
class eTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    eTextEdit(eParameter &param, QWidget *parent=eNULL);

Q_SIGNALS:
    void            onParameterChanged(const eParameter &param);

private Q_SLOTS:
    void            _onTextChanged();

private:
    eParameter &    m_param;
};

// Single-line edit, which writes its text to
// parameter passed in constructor.
class eLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    eLineEdit(eParameter &param, QWidget *parent=eNULL);

Q_SIGNALS:
    void            onParameterChanged(const eParameter &param);

private Q_SLOTS:
    void            _onTextChanged(const QString &text);

private:
    eParameter &    m_param;
};

// Encapsulates a line-edit, which displays the
// user-name of the linked operator, together
// with a button to select the operator and a
// button to jump to the selected operator.
class eLinkFrame : public QWidget
{
    Q_OBJECT

public:
    enum Type
    {
        TYPE_OPERATOR,
        TYPE_ANIMATION
    };

public:
    eLinkFrame(eParameter &param, Type type, QWidget *parent=eNULL);

Q_SIGNALS:
    void            onParameterChanged(const eParameter &param);
    void            onGotoOperator(eID opId);

private Q_SLOTS:
    void            _onSelectGlobally();
    void            _onSelectLocally();
    void            _onClearClicked();
    void            _onGotoClicked();

private:
    void            _linkOperatorWithId(eID opId);

private:
    static eBool    _sortOpsByName(const eIOperator * const &op0, const eIOperator * const &op1);

private:
    Type            m_type;
    QLineEdit *     m_edit;
    QPushButton *   m_selLocBtn;
    eParameter &    m_param;
};

// Encapsulates three or four track edits (depending
// on type of parameter: RGB or RGBA), together with
// a button to select the color.
class eColorFrame : public QWidget
{
    Q_OBJECT

public:
    eColorFrame(eParameter &param, QWidget *parent=eNULL);
    virtual ~eColorFrame();

private:
    virtual void    timerEvent(QTimerEvent *te);

Q_SIGNALS:
    void            onParameterChanged(const eParameter &param);

private Q_SLOTS:
    void            _onSelectLocally();
    void            _updateEditColors();

private:
    eInt            m_timerId;
    eParameter &    m_param;
    eTrackEdit *    m_edits[4];
};

// Used for displaying source code of transform shaders
// with syntax highlighting.
class eSyntaxHighlighter : public QSyntaxHighlighter
{
public:
    eSyntaxHighlighter(QTextDocument *document);

private:
    virtual void    highlightBlock(const QString &text);

private:
    QTextCharFormat m_fmtCommand;
    QTextCharFormat m_fmtComment;
    QTextCharFormat m_fmtDigit;
};

// Encapsulates a text editor to edit source code
// and a compile button.
class eTShaderCodeEditor : public QWidget
{
    Q_OBJECT

public:
    eTShaderCodeEditor(eParameter &param, QWidget *parent=eNULL);

Q_SIGNALS:
    void                    onParameterChanged(const eParameter &param);

private Q_SLOTS:
    void                    _onCompile();

private:
    QTextEdit *             m_edit;
    eSyntaxHighlighter *    m_highlighter;
    QPushButton *           m_compileBtn;
    eParameter &            m_param;
};

// Frame showing all widgets for choosing parameters
// to animate a parameter.
class eAnimateFrame : public QWidget
{
    Q_OBJECT

public:
    eAnimateFrame(eParameter &param, QWidget *parent=eNULL);

Q_SIGNALS:
    void            onParameterChanged(const eParameter *param);

private Q_SLOTS:    
    void            _onAnimChannelChanged(int index);
    void            _onOffsetChanged(const QString &off);
    void            _onScaleChanged(const QString &scale);

private:
    eParameter &    m_param;
};

// Button for enabling/disabling animation of a parameter
// and creating/deleting all the related widgets.
class eAnimateSwitchButton : public QPushButton
{
    Q_OBJECT

public:
    eAnimateSwitchButton(eParameter &param, QLayout *layout, QWidget *parent=eNULL);

private Q_SLOTS:
    void            _onClicked();

private:
    void            _openAnimationWidgets();
    void            _closeAnimationWidgets();

private:
    eParameter &    m_param;
    eAnimateFrame * m_animFrame;
    QLayout *       m_layout;
};

#endif // PARAM_WIDGETS_HPP