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

#include "paramwidgets.hpp"
#include "findopdlg.hpp"

#include <QtGui/QDoubleValidator>
#include <QtGui/QIntValidator>
#include <QtGui/QColorDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QWheelEvent>
#include <QtGui/QMenu>

static const eInt FLOAT_PRECISION = 3;

eTrackEdit::eTrackEdit(eParameter &param, eU32 component, QWidget *parent) : QLineEdit(parent),
    m_component(component),
    m_param(param),
    m_ctrlDown(eFALSE),
    m_shiftDown(eFALSE)
{
    eASSERT(param.isIntType() || param.isFloatType());
    eASSERT(component < param.getComponentCount());

    // Setup signals and slots.
    connect(this, SIGNAL(textEdited(const QString &)), this, SLOT(_onTextEdited(const QString &)));

    // Setup validators and set text.
    eF32 min, max;

    param.getMinMax(min, max);

    if (param.isIntType())
    {
        setValidator(new QIntValidator((eInt)min, (eInt)max, this));
        setText(eIntToStr(_getInt()));
    }
    else if (param.isFloatType())
    {
        QDoubleValidator *dv = new QDoubleValidator(min, max, 3, this);
        eASSERT(dv != eNULL);
        dv->setLocale(QLocale::C);
        dv->setNotation(QDoubleValidator::StandardNotation);
        setValidator(dv);
        setText(QString::number(_getFloat(), 'f', FLOAT_PRECISION));
    }
    else
    {
        // Error, unsupported type.
        eASSERT(eFALSE);
    }

    m_timerId = startTimer(50);
}

eTrackEdit::~eTrackEdit()
{
    killTimer(m_timerId);
}

void eTrackEdit::_onTextEdited(const QString &text)
{
    // Write value back to parameter, but only
    // if input value is acceptable regarding
    // set validator.
    if (hasAcceptableInput())
    {
        if (m_param.isIntType())
        {
            _setInt(text.toInt());
        }
        else if (m_param.isFloatType())
        {
            _setFloat((eF32)text.toFloat());
        }

        // Update parameter's owning operator and
        // Q_EMIT changed signal.
        m_param.setChanged(eTRUE);
        m_param.getOwnerOp()->setChanged();

        Q_EMIT onParameterChanged(m_param);
    }
}

eBool eTrackEdit::_changeValue(eF32 factor)
{
    if (m_ctrlDown)
    {
        // [Control] => change slow (0.1x).
        factor *= 0.1f;
    }
    else if (m_shiftDown)
    {
        // [Shift] => change fast (10x).
        factor *= 10.0f;
    }

    // Calculate new parameter value.
    QString newText;

    if (m_param.isIntType())
    {
        // Is value to small for change?
        if (eAbs(factor) < 1.0f)
        {
            return eFALSE;
        }

        newText.setNum(_getInt()+(eInt)factor);
    }
    else if (m_param.isFloatType())
    {
        newText.setNum(_getFloat()+0.01f*factor, 'f', FLOAT_PRECISION);
    }

    // Validate new text to have correct format
    // and to be in requested range.
    eInt pos;

    if (validator()->validate(newText, pos) == QValidator::Acceptable)
    {
        setText(newText);
    }

    Q_EMIT textEdited(text());
    return eTRUE;
}

eInt eTrackEdit::_getInt() const
{
    return eRect(m_param.getValue().ixyxy)[m_component];
}

eF32 eTrackEdit::_getFloat() const
{
    return eVector4(m_param.getValue().fxyzw)[m_component];
}

void eTrackEdit::_setInt(eInt value)
{
    eRect r = m_param.getValue().ixyxy;
    r[m_component] = value;
    m_param.getValue().ixyxy = r;
}

void eTrackEdit::_setFloat(eF32 value)
{
    eVector4 v = m_param.getValue().fxyzw;
    v[m_component] = value;
    m_param.getValue().fxyzw = v;
}

void eTrackEdit::wheelEvent(QWheelEvent *we)
{
    if (hasFocus())
    {
        _changeValue(eSign(we->delta()));
        we->accept();
    }
    else
    {
        QLineEdit::wheelEvent(we);
    }
}

void eTrackEdit::keyPressEvent(QKeyEvent *ke)
{
    QLineEdit::keyPressEvent(ke);

    if (ke->key() == Qt::Key_Control)
    {
        m_ctrlDown = eTRUE;
    }
    else if (ke->key() == Qt::Key_Shift)
    {
        m_shiftDown = eTRUE;
    }
}

void eTrackEdit::keyReleaseEvent(QKeyEvent *ke)
{
    QLineEdit::keyReleaseEvent(ke);

    m_ctrlDown   = eFALSE;
    m_shiftDown  = eFALSE;
}

void eTrackEdit::mouseMoveEvent(QMouseEvent *me)
{
    QLineEdit::mouseMoveEvent(me);

    if (me->buttons() & Qt::RightButton)
    {
        const QPoint diff = me->pos()-m_mouseDownPos;

        if (eAbs(diff.x()) > 1 && _changeValue(diff.x()/2))
        {
            QCursor::setPos(mapToGlobal(m_mouseDownPos));
            m_valueChanged = eTRUE;
        }
    }
}

void eTrackEdit::mousePressEvent(QMouseEvent *me)
{
    QLineEdit::mousePressEvent(me);
    m_mouseDownPos = me->pos();
}

void eTrackEdit::mouseReleaseEvent(QMouseEvent *me)
{
    QLineEdit::mouseReleaseEvent(me);

    const QPoint diff = me->pos()-m_mouseDownPos;

    if (m_valueChanged)
    {
        QLineEdit::setContextMenuPolicy(Qt::PreventContextMenu);
        m_valueChanged = eFALSE;
    }
    else
    {
        QLineEdit::setContextMenuPolicy(Qt::DefaultContextMenu);
    }
}

void eTrackEdit::timerEvent(QTimerEvent *te)
{
    QLineEdit::timerEvent(te);

    if (m_param.isAnimated())
    {
        if (m_param.isIntType())
        {
            setText(eIntToStr(_getInt()));
        }
        else if (m_param.isFloatType())
        {
            setText(QString::number(_getFloat(), 'f', FLOAT_PRECISION));
        }
    }
}

void eTrackEdit::focusInEvent(QFocusEvent *fe)
{
    QLineEdit::focusInEvent(fe);
    setFocusPolicy(Qt::WheelFocus);
}

void eTrackEdit::focusOutEvent(QFocusEvent *fe)
{
    QLineEdit::focusOutEvent(fe);
    setFocusPolicy(Qt::StrongFocus);
}

eBoolButton::eBoolButton(eParameter &param, QWidget *parent) : QPushButton(parent),
    m_param(param)
{
    eASSERT(param.getType() == eParameter::TYPE_BOOL);

    _updateCaption();
    connect(this, SIGNAL(clicked()), this, SLOT(_onClicked()));
}

void eBoolButton::_updateCaption()
{
    setText(m_param.getValue().boolean ? "Yes" : "No");
}

void eBoolButton::_onClicked()
{
    // Set next item and update cycle index.
    m_param.getValue().boolean = !m_param.getValue().boolean;
    _updateCaption();

    m_param.setChanged(eTRUE);
    m_param.getOwnerOp()->setChanged();

    Q_EMIT onParameterChanged(m_param);
}

eComboBox::eComboBox(eParameter &param, QWidget *parent) : QComboBox(parent),
    m_param(param)
{
    eASSERT(param.getDescriptionItems().size() > 0);
    eASSERT(param.getValue().enumSel < (eInt)param.getDescriptionItems().size());

    for (eU32 i=0; i<param.getDescriptionItems().size(); i++)
    {
        addItem(QString(*param.getDescriptionItems()[i]));
    }

    setCurrentIndex(param.getValue().enumSel);
    connect(this, SIGNAL(activated(int)), this, SLOT(_onActivated(int)));
}

void eComboBox::_onActivated(int index)
{
    m_param.getValue().enumSel = index;

    m_param.setChanged(eTRUE);
    m_param.getOwnerOp()->setChanged();

    Q_EMIT onParameterChanged(m_param);
}

eFlagButton::eFlagButton(eParameter &param, eU32 flagIndex, QWidget *parent) : QPushButton(parent),
    m_param(param),
    m_flagIndex(flagIndex)
{
    eASSERT(param.getType() == eParameter::TYPE_FLAGS);
    eASSERT(param.getDescriptionItems().size() > 0);
    eASSERT(flagIndex < param.getDescriptionItems().size());

    setText(QString(*m_param.getDescriptionItems()[flagIndex]));
    setCheckable(true);
    _updateDownState();

    connect(this, SIGNAL(clicked()), this, SLOT(_onClicked()));
}

void eFlagButton::_updateDownState()
{
    setChecked(eGetBit(m_param.getValue().flags, m_flagIndex));
}

void eFlagButton::_onClicked()
{
    // Toggle bit flag and set button to down, or not.
    eToggleBit(m_param.getValue().flags, m_flagIndex);
    _updateDownState();

    m_param.setChanged(eTRUE);
    m_param.getOwnerOp()->setChanged();

    Q_EMIT onParameterChanged(m_param);
}

eTextEdit::eTextEdit(eParameter &param, QWidget *parent) :
    m_param(param)
{
    eASSERT(param.getType() == eParameter::TYPE_TEXT);

    setFixedHeight(135);
    setPlainText(m_param.getValue().text);

    connect(this, SIGNAL(textChanged()), this, SLOT(_onTextChanged()));
}

void eTextEdit::_onTextChanged()
{
    eStrNCopy(m_param.getValue().text, toPlainText().toAscii(), eParameter::MAX_TEXT_LENGTH);

    m_param.setChanged(eTRUE);
    m_param.getOwnerOp()->setChanged();

    Q_EMIT onParameterChanged(m_param);
}

eLineEdit::eLineEdit(eParameter &param, QWidget *parent) :
    m_param(param)
{
    eASSERT(param.getType() == eParameter::TYPE_STRING ||
            param.getType() == eParameter::TYPE_SYNTH  ||
            param.getType() == eParameter::TYPE_FILE);

    setText(m_param.getValue().string);
    setMaxLength(eParameter::MAX_TEXT_LENGTH);

    connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(_onTextChanged(const QString &)));
}

void eLineEdit::_onTextChanged(const QString &text)
{
    eStrNCopy(m_param.getValue().string, text.toAscii(), eParameter::MAX_TEXT_LENGTH);

    m_param.setChanged(eTRUE);
    m_param.getOwnerOp()->setChanged();

    Q_EMIT onParameterChanged(m_param);
}

eLinkFrame::eLinkFrame(eParameter &param, Type type, QWidget *parent) : QWidget(parent),
    m_param(param),
    m_type(type)
{
    // Create line-edit, which displays
    // loaded operators name.
    m_edit = new QLineEdit;
    eASSERT(m_edit != eNULL);
    m_edit->setReadOnly(true);

    eID opId;

    if (type == TYPE_OPERATOR)
    {
        opId = param.getValue().linkedOpId;
    }
    else
    {
        opId = param.getAnimationPathOpId();
    }

    const eIOperator *linkedOp = eDemoData::findOperator(opId);
    m_edit->setText(linkedOp ? QString(linkedOp->getUserName()) : "<< Empty >>");

    // Create clear, select and goto buttons.
    QPushButton *clearBtn = new QPushButton("Clear");
    eASSERT(clearBtn != eNULL);
    clearBtn->setToolTip("Clears currently linked operator");
    clearBtn->setFixedWidth(40);
    connect(clearBtn, SIGNAL(clicked()), this, SLOT(_onClearClicked()));

    QPushButton *selGlobBtn = new QPushButton("Find");
    eASSERT(selGlobBtn != eNULL);
    selGlobBtn->setToolTip("Select globally (from any page)");
    selGlobBtn->setFixedWidth(40);
    connect(selGlobBtn, SIGNAL(clicked()), this, SLOT(_onSelectGlobally()));

    m_selLocBtn = new QPushButton("Pick");
    eASSERT(m_selLocBtn != eNULL);
    m_selLocBtn->setToolTip("Select locally (from this page)");
    m_selLocBtn->setFixedWidth(40);
    connect(m_selLocBtn, SIGNAL(clicked()), this, SLOT(_onSelectLocally()));

    QPushButton *gotoBtn = new QPushButton("Goto");
    eASSERT(gotoBtn != eNULL);
    gotoBtn->setToolTip("Goto linked operator");
    gotoBtn->setFixedWidth(40);
    connect(gotoBtn, SIGNAL(clicked()), this, SLOT(_onGotoClicked()));

    // Create layout with above widgets.
    QHBoxLayout *hbl = new QHBoxLayout;
    eASSERT(hbl != eNULL);
    hbl->setContentsMargins(0, 0, 0, 0);
    hbl->addWidget(m_edit);
    hbl->addWidget(clearBtn);
    hbl->addWidget(selGlobBtn);
    hbl->addWidget(m_selLocBtn);
    hbl->addWidget(gotoBtn);

    // Setup frame properties.
    setLayout(hbl);
    setFixedHeight(25);
}

void eLinkFrame::_onSelectGlobally()
{
    QStringList allowedLinks;
    
    if (m_type == TYPE_OPERATOR)
    {
        const eStringPtrArray &al = m_param.getAllowedLinks().getAllAllowed();

        for (eU32 i=0; i<al.size(); i++)
        {
            allowedLinks.append(QString(*al[i]));
        }
    }
    else
    {
        allowedLinks.append("Path");
    }

    eFindOpDlg dlg(allowedLinks);

    if (dlg.exec() == QDialog::Accepted)
    {
        _linkOperatorWithId(dlg.getSelectedOpId());
    }
}

void eLinkFrame::_onSelectLocally()
{
    // Create menu with all named operators,
    // lying on the owner page of the operator,
    // owning the parameter.
    QMenu menu;

    const eOperatorPage *page = m_param.getOwnerOp()->getOwnerPage();
    eASSERT(page != eNULL);

    QStringList allowedLinks;

    if (m_type == TYPE_OPERATOR)
    {
        const eStringPtrArray &al = m_param.getAllowedLinks().getAllAllowed();

        for (eU32 i=0; i<al.size(); i++)
        {
            allowedLinks.append(QString(*al[i]));
        }
    }
    else
    {
        allowedLinks.append("Path");
    }

    // Find operator for the menu.
    eIOpConstPtrArray ops;

    for (eU32 i=0; i<page->getOperatorCount(); i++)
    {
        const eIOperator *op = page->getOperatorByIndex(i);
        eASSERT(op != eNULL);

        if (allowedLinks.contains("All") ||
            allowedLinks.contains(QString(op->getType())) ||
            allowedLinks.contains(QString(op->getCategory())))
        {
            if (op->getRealType() != "Misc : Load" && op->getUserName() != "")
            {
                ops.append(op);
            }
        }
    }

    ops.sort(_sortOpsByName);

    for (eU32 i=0; i<ops.size(); i++)
    {
        const eIOperator *op = ops[i];
        eASSERT(op != eNULL);

        menu.addAction(QString(op->getUserName()))->setData(op->getId());
    }

    // Show menu below select-button.
    if (menu.actions().size() > 0)
    {
        QAction *act = menu.exec(mapToGlobal(m_selLocBtn->geometry().bottomLeft()));

        if (act)
        {
            _linkOperatorWithId(act->data().toInt());
        }
    }
}

void eLinkFrame::_onClearClicked()
{
    _linkOperatorWithId(eNOID);
}

void eLinkFrame::_onGotoClicked()
{
    eID opId;

    if (m_type == TYPE_OPERATOR)
    {
        opId = m_param.getValue().linkedOpId;
    }
    else
    {
        opId = m_param.getAnimationPathOpId();
    }

    Q_EMIT onGotoOperator(opId);
}

void eLinkFrame::_linkOperatorWithId(eID opId)
{
    if (m_type == TYPE_OPERATOR)
    {
        m_param.getValue().linkedOpId = opId;
    }
    else
    {
        m_param.setAnimationPathOpId(opId);
    }

    const eIOperator *linkedOp = eDemoData::findOperator(opId);
    m_edit->setText(linkedOp ? QString(linkedOp->getUserName()) : "<< Empty >>");

    m_param.getOwnerOp()->getOwnerPage()->updateLinks();
    m_param.setChanged(eTRUE);
    m_param.getOwnerOp()->setChanged();

    Q_EMIT onParameterChanged(m_param);
}

eBool eLinkFrame::_sortOpsByName(const eIOperator * const &op0, const eIOperator * const &op1)
{
    eASSERT(op0 != eNULL);
    eASSERT(op1 != eNULL);

    const QString s0(op0->getUserName());
    const QString s1(op1->getUserName());

    return (s0.compare(s1, Qt::CaseInsensitive) > 0);
}

eColorFrame::eColorFrame(eParameter &param, QWidget *parent) : QWidget(parent),
    m_param(param)
{
    eASSERT(param.getType() == eParameter::TYPE_RGB ||
            param.getType() == eParameter::TYPE_RGBA);

    // Create track edits and button.
    QHBoxLayout *hbl = new QHBoxLayout;
    eASSERT(hbl != eNULL);
    hbl->setContentsMargins(0, 0, 0, 0);

    for (eU32 i=0; i<param.getComponentCount(); i++)
    {
        m_edits[i] = new eTrackEdit(param, i);
        eASSERT(m_edits[i] != eNULL);
        hbl->addWidget(m_edits[i]);

        connect(m_edits[i], SIGNAL(onParameterChanged(const eParameter &)),
                this, SLOT(_updateEditColors()));
        connect(m_edits[i], SIGNAL(onParameterChanged(const eParameter &)),
                this, SIGNAL(onParameterChanged(const eParameter &)));
    }

    QPushButton *selBtn = new QPushButton("...");
    eASSERT(selBtn != eNULL);
    selBtn->setFixedWidth(25);
    hbl->addWidget(selBtn);

    // Setup frame properties.
    setLayout(hbl);
    setFixedHeight(25);

    _updateEditColors();
    connect(selBtn, SIGNAL(clicked()), this, SLOT(_onSelectLocally()));
    m_timerId = startTimer(50);
}

eColorFrame::~eColorFrame()
{
    killTimer(m_timerId);
}

void eColorFrame::timerEvent(QTimerEvent *te)
{
    QWidget::timerEvent(te);

    if (m_param.isAnimated())
    {
        _updateEditColors();
    }
}

void eColorFrame::_onSelectLocally()
{
    eFloatColor &pc = m_param.getValue().color;
    const QColor col = QColorDialog::getColor(QColor::fromRgbF(pc.r, pc.g, pc.b));

    if (col.isValid())
    {
        // Write color back to parameter.
        pc.r = col.redF();
        pc.g = col.greenF();
        pc.b = col.blueF();

        // Update text and color of edits.
        m_edits[0]->setText(QString::number(pc.r, 'f', FLOAT_PRECISION));
        m_edits[1]->setText(QString::number(pc.g, 'f', FLOAT_PRECISION));
        m_edits[2]->setText(QString::number(pc.b, 'f', FLOAT_PRECISION));

        // Set operator to changed.
        m_param.setChanged(eTRUE);
        m_param.getOwnerOp()->setChanged();

        _updateEditColors();
        Q_EMIT onParameterChanged(m_param);
    }
}

void eColorFrame::_updateEditColors()
{
    for (eU32 i=0; i<m_param.getComponentCount(); i++)
    {
        const eColor color = m_param.getValue().color;

        // Set background color.
        QPalette pal = m_edits[i]->palette();
        pal.setColor(QPalette::Base, QColor(color.toArgb()));

        // Set text color on track-edit depending on
        // intensity of color (dark color => light
        // text, light color => dark text).
        if (color.grayScale() > 128)
        {
            pal.setColor(QPalette::Text, Qt::black);
        }
        else
        {
            pal.setColor(QPalette::Text, Qt::white);
        }

        m_edits[i]->setPalette(pal);
    }
}

eTShaderCodeEditor::eTShaderCodeEditor(eParameter &param, QWidget *parent) : QWidget(parent),
    m_param(param)
{
    eASSERT(param.getType() == eParameter::TYPE_TSHADERCODE);

    // Create text-edit, which displays code.
    m_edit = new QTextEdit;
    eASSERT(m_edit != eNULL);
    m_edit->setFont(QFont("Courier New", 10, QFont::Normal));
    m_edit->setText(m_param.getValue().sourceCode);

    m_highlighter = new eSyntaxHighlighter(m_edit->document());
    eASSERT(m_highlighter != eNULL);

    m_compileBtn = new QPushButton("Compile");
    eASSERT(m_compileBtn != eNULL);
    connect(m_compileBtn, SIGNAL(clicked()), this, SLOT(_onCompile()));

    // Create layout with above widgets.
    QVBoxLayout *vbl = new QVBoxLayout;
    eASSERT(vbl != eNULL);
    vbl->setContentsMargins(0, 0, 0, 0);
    vbl->addWidget(m_edit);
    vbl->addWidget(m_compileBtn);
    
    // Setup frame properties.
    setLayout(vbl);
    setFixedHeight(300);
}

void eTShaderCodeEditor::_onCompile()
{
    const QByteArray &ba = m_edit->toPlainText().toAscii();

    eStrNCopy(m_param.getValue().sourceCode, ba.constData(), eParameter::MAX_TEXT_LENGTH);
    m_param.setChanged(eTRUE);
    m_param.getOwnerOp()->setChanged();

    Q_EMIT onParameterChanged(m_param);
}

eSyntaxHighlighter::eSyntaxHighlighter(QTextDocument *document) : QSyntaxHighlighter(document)
{
    m_fmtCommand.setForeground(QColor(128, 128, 255));
    m_fmtCommand.setFontWeight(QFont::Bold);

    m_fmtComment.setForeground(QColor(64, 64, 64));
    m_fmtComment.setFontWeight(QFont::Normal);

    m_fmtDigit.setForeground(QColor(128, 0, 128));
    m_fmtDigit.setFontWeight(QFont::Normal);
}

void eSyntaxHighlighter::highlightBlock(const QString &text)
{
    for (eInt i=0; i<text.length(); i++)
    {
        for (eU32 j=0; j<eTShaderCompiler::getInstructionCount(); j++)
        {
            const QString instrName = QString(eTShaderCompiler::getInstructionName(j))+" ";

            if (text.mid(i, instrName.length()) == instrName)
            {
                setFormat(i, instrName.length()-i, m_fmtCommand);
                break;
            }
        }

        if (text[i].isDigit())
        {
            setFormat(i, 1, m_fmtDigit);
        }

        if (text.mid(i, 2) == "//") 
        {
            setFormat(i, text.length()-i, m_fmtComment);
            break;
        } 
    }
}

eAnimateFrame::eAnimateFrame(eParameter &param, QWidget *parent) : QWidget(parent),
    m_param(param)
{
    setFixedHeight(25);

    QHBoxLayout *hbl = new QHBoxLayout(this);
    eASSERT(hbl != eNULL);
    hbl->setContentsMargins(0, 0, 0, 0);

    QLabel *lblPath = new QLabel("Path");
    eASSERT(lblPath != eNULL);
    lblPath->setAlignment(Qt::AlignRight);
    lblPath->setFixedWidth(40);
    hbl->addWidget(lblPath, 0, Qt::AlignVCenter);


    QLineEdit *editOffset = new QLineEdit(QString::number(m_param.getAnimationTimeOffset()));
    hbl->addWidget(editOffset);

    QLineEdit *editScale = new QLineEdit(QString::number(m_param.getAnimationTimeScale()));
    hbl->addWidget(editScale);

    QComboBox *cb = new QComboBox;
    eASSERT(cb != eNULL);
    cb->setFixedWidth(85);
    cb->addItem("Rotation");
    cb->addItem("Translation");
    cb->addItem("Scaling");
    cb->setCurrentIndex(m_param.getAnimationChannel());
    hbl->addWidget(cb, 0);

    hbl->addWidget(new eLinkFrame(param, eLinkFrame::TYPE_ANIMATION), 1);

    connect(cb, SIGNAL(currentIndexChanged(int)), this, SLOT(_onAnimChannelChanged(int)));

    connect(editOffset, SIGNAL(textChanged(const QString &)), this, SLOT(_onOffsetChanged(const QString &)));
    connect(editScale, SIGNAL(textChanged(const QString &)), this, SLOT(_onScaleChanged(const QString &)));
}

void eAnimateFrame::_onAnimChannelChanged(int index)
{
    m_param.setAnimationChannel((eParameter::AnimationChannel)index);
    m_param.setChanged(eTRUE);
    m_param.getOwnerOp()->setChanged();
}

void eAnimateFrame::_onOffsetChanged(const QString &off)
{
    m_param.setAnimationTimeOffset(off.toFloat());
}

void eAnimateFrame::_onScaleChanged(const QString &scale)
{
    m_param.setAnimationTimeScale(scale.toFloat());
}

eAnimateSwitchButton::eAnimateSwitchButton(eParameter &param, QLayout *layout, QWidget *parent) : QPushButton(parent),
    m_param(param),
    m_layout(layout)
{
    setText("A");
    setFixedWidth(25);
    setCheckable(true);

    connect(this, SIGNAL(clicked()), this, SLOT(_onClicked()));

    m_animFrame = new eAnimateFrame(param, this);
    eASSERT(m_animFrame != eNULL);
    layout->addWidget(m_animFrame);

    if (param.isAnimated())
    {
        m_animFrame->show();
        setChecked(true);
    }
    else
    {
        m_animFrame->hide();
        setChecked(false);
    }
}

void eAnimateSwitchButton::_onClicked()
{
    if (!m_param.isAnimated())
    {
        m_param.setAnimated(eTRUE);
        m_animFrame->show();
    }
    else
    {
        m_param.setAnimated(eFALSE);
        m_animFrame->hide();
    }
}