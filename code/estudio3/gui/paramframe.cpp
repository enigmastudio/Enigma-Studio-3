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

#include <QtGui/QFontMetrics>
#include <QtGui/QVBoxLayout>
#include <QtGui/QScrollBar>
#include <QtGui/QLabel>

#include "paramframe.hpp"
#include "paramwidgets.hpp"

eParameterFrame::eParameterFrame(QWidget *parent) : QScrollArea(parent),
    m_opId(eNOID)
{
    setWidgetResizable(true);
    setWidget(new QFrame(viewport()));

    m_layout = new QVBoxLayout(widget());
    eASSERT(m_layout != eNULL);
    m_layout->setSpacing(5);
    m_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
}

eParameterFrame::~eParameterFrame()
{
    _clearWidgets();
}

void eParameterFrame::setOperator(eID opId)
{
    if (opId != m_opId)
    {
        m_opId = opId;
        _createWidgets();
    }
}

eID eParameterFrame::getOperator() const
{
    return m_opId;
}

void eParameterFrame::_createWidgets()
{
    _clearWidgets();

    // If operator ID is invalid, no new widgets
    // have to be created.
    if (!eDemoData::existsOperator(m_opId))
    {
        return;
    }

    // Create widgets for non parameter properties.
    eIOperator *op = eDemoData::findOperator(m_opId);
    eASSERT(op != eNULL);

    _addDefaultParams(op);

    if (op->getParameterCount() == 0)
    {
        return;
    }

    _addMainSeparator();

    // Get width of widest description label.
    eInt maxLabelWidth = 0;

    for (eU32 i=0; i<op->getParameterCount(); i++)
    {
        const eParameter &p = op->getParameter(i);

        // Only regard parameter if it's not a label,
        // because labels don't have a description
        // label in front of the parameter.
        if (p.getType() != eParameter::TYPE_LABEL)
        {
            const eInt curWidth = fontMetrics().width(QString(p.getName()));

            if (maxLabelWidth < curWidth)
            {
                maxLabelWidth = curWidth;
            }
        }
    }

    // Create parameter widgets.
    for (eU32 i=0; i<op->getParameterCount(); i++)
    {
        eParameter &p = op->getParameter(i);

        if (p.getType() == eParameter::TYPE_LABEL)
        {
            // Parameter is a label => no
            // description label needed.
            _addSeparatorLabel(p.getValue().label, i+2);
        }
        else
        {
            // Create description label.
            QLabel *descrLabel = new QLabel(QString(p.getName()));
            eASSERT(descrLabel != eNULL);
            descrLabel->setFixedWidth(maxLabelWidth);

            // Create horizontal layout for
            // parameter widgets.
            QHBoxLayout *hbl = new QHBoxLayout;
            eASSERT(hbl != eNULL);
            m_layout->addLayout(hbl);
            hbl->setSpacing(5);
            hbl->addWidget(descrLabel);

            switch (p.getType())
            {
                case eParameter::TYPE_BOOL:
                {
                    hbl->addWidget(new eBoolButton(p));
                    break;
                }

                case eParameter::TYPE_ENUM:
                {
                    hbl->addWidget(new eComboBox(p), 1);
                    break;
                }

                case eParameter::TYPE_FLAGS:
                {
                    for (eU32 j=0; j<p.getDescriptionItems().size(); j++)
                    {
                        hbl->addWidget(new eFlagButton(p, j), 1);
                    }

                    break;
                }

                case eParameter::TYPE_TEXT:
                {
                    hbl->addWidget(new eTextEdit(p), 1);
                    break;
                }

                case eParameter::TYPE_FILE:
                case eParameter::TYPE_STRING:
                case eParameter::TYPE_SYNTH:
                {
                    hbl->addWidget(new eLineEdit(p), 1);
                    break;
                }

                case eParameter::TYPE_TSHADERCODE:
                {
                    hbl->addWidget(new eTShaderCodeEditor(p), 1);
                    break;
                }

                case eParameter::TYPE_LINK:
                {
                    eLinkFrame *linkFrame = new eLinkFrame(p, eLinkFrame::TYPE_OPERATOR);
                    eASSERT(linkFrame != eNULL);
                    hbl->addWidget(linkFrame, 1);
                    connect(linkFrame, SIGNAL(onGotoOperator(eID)), this, SIGNAL(onGotoOperator(eID)));
                    break;
                }

                case eParameter::TYPE_RGB:
                case eParameter::TYPE_RGBA:
                {
                    hbl->addWidget(new eColorFrame(p), 1);
                    break;
                }

                default:
                {
                    for (eU32 j=0; j<p.getComponentCount(); j++)
                    {
                        hbl->addWidget(new eTrackEdit(p, j), 1);
                    }

                    break;
                }
            }

            // Make connections for parameter changed
            // => operator changed signal.
            for (eInt i=1; i<hbl->count(); i++)
            {
                connect(hbl->itemAt(i)->widget(),
                        SIGNAL(onParameterChanged(const eParameter &)),
                        this, SLOT(_onParameterChanged(const eParameter &)));
            }

            // Create "animate" button for animatable parameters.
            if (p.isAnimatable())
            {
                hbl->addWidget(new eAnimateSwitchButton(p, m_layout));
            }
        }
    }
}

// Removes all widgets and sub-layouts
// from main layout.
void eParameterFrame::_clearWidgets()
{
    _clearLayout(m_layout);
}

// Removes all widgets and sub-layouts from
// the given layout. For each sub-layout this
// function is called again to clean up properly.
void eParameterFrame::_clearLayout(QLayout *layout)
{
    eASSERT(layout != eNULL);

    QLayoutItem *item = eNULL;

    while ((item = layout->itemAt(0)) != eNULL)
    {
        if (item->layout())
        {
            _clearLayout(item->layout());
            delete item->layout();
        }
        else if (item->widget())
        {
            delete item->widget();
        }

        layout->removeItem(item);
    }
}

void eParameterFrame::_addMainSeparator()
{
    QFrame *sep0 = new QFrame(widget());
    eASSERT(sep0 != eNULL);
    sep0->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    QFrame *sep1 = new QFrame(widget());
    eASSERT(sep1 != eNULL);
    sep1->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    QVBoxLayout *vbl = new QVBoxLayout;
    eASSERT(vbl != eNULL);
    vbl->setSpacing(1);
    vbl->addWidget(sep0);
    vbl->addWidget(sep1);

    m_layout->addLayout(vbl);
}

void eParameterFrame::_addDefaultParams(const eIOperator *op)
{
    eASSERT(op != eNULL);

    QPushButton *defaultNameBtn = new QPushButton("@", widget());
    eASSERT(defaultNameBtn != eNULL);
    defaultNameBtn->setFixedWidth(20);

    QPushButton *bypassBtn = new QPushButton("Bypass", widget());
    eASSERT(bypassBtn != eNULL);
    bypassBtn->setCheckable(true);
    bypassBtn->setChecked(op->getBypassed());
    bypassBtn->setFixedWidth(50);

    QPushButton *hideBtn = new QPushButton("Hide", widget());
    eASSERT(hideBtn != eNULL);
    hideBtn->setCheckable(true);
    hideBtn->setChecked(op->getHidden());
    hideBtn->setFixedWidth(40);

    QLineEdit *nameEdit = new QLineEdit(QString(op->getUserName()), widget());
    eASSERT(nameEdit != eNULL);
	this->m_nameField = nameEdit;

    connect(defaultNameBtn, SIGNAL(clicked()), this, SLOT(_onDefaultNameClicked()));
    connect(bypassBtn, SIGNAL(clicked(bool)), this, SLOT(_onBypassClicked(bool)));
    connect(hideBtn, SIGNAL(clicked(bool)), this, SLOT(_onHideClicked(bool)));
    connect(nameEdit, SIGNAL(textEdited(const QString &)), this, SLOT(_onNameChanged(const QString &)));

    QHBoxLayout *hbl = new QHBoxLayout;
    eASSERT(hbl != eNULL);
    hbl->addWidget(new QLabel(QString("<b>")+op->getRealType()+"</b>"), 0, 0);
    hbl->addWidget(defaultNameBtn, 0);
    hbl->addWidget(nameEdit, 1);
    hbl->addWidget(bypassBtn, 0);
    hbl->addWidget(hideBtn, 0);

    m_layout->addLayout(hbl);
}

// Adds an separator to the parameter window.
// The separator is put into a horizontal
// layout and consists of three labels.
void eParameterFrame::_addSeparatorLabel(const QString &caption, eU32 row)
{
    eASSERT(m_layout != eNULL);

    QFrame *sepLeft = new QFrame(widget());
    eASSERT(sepLeft != eNULL);
    sepLeft->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    QFrame *sepRight = new QFrame(widget());
    eASSERT(sepRight != eNULL);
    sepRight->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    QHBoxLayout *hbl = new QHBoxLayout;
    eASSERT(hbl != eNULL);
    hbl->addWidget(sepLeft, 1);
    hbl->addWidget(new QLabel(caption), 0);
    hbl->addWidget(sepRight, 1);

    m_layout->addLayout(hbl);
}

void eParameterFrame::_onBypassClicked(bool checked)
{
    eIOperator *op = eDemoData::findOperator(m_opId);
    eASSERT(op != eNULL);
    op->setBypassed(checked);
    op->getOwnerPage()->updateLinks();

    Q_EMIT onOperatorChanged(eNULL);
}

static eU32 newNameCounter = 1;
void eParameterFrame::_onDefaultNameClicked()
{
	QLineEdit * nameField = (QLineEdit *) this->m_nameField;
    eIOperator *op = eDemoData::findOperator(m_opId);
    eASSERT(op != eNULL);
	if(op->getUserName() == eString("")) {
		eString s = op->getCategory() + eString(":") + op->getName() + eString("_") + eIntToStr(newNameCounter++);
		op->setUserName(s);
	}
	else
		op->setUserName(eString(""));
    op->getOwnerPage()->updateLinks();
	nameField->setText(QString(op->getUserName()));

    Q_EMIT onOperatorChanged(eNULL);
    setWindowModified(true);
}


void eParameterFrame::_onHideClicked(bool checked)
{
    eIOperator *op = eDemoData::findOperator(m_opId);
    eASSERT(op != eNULL);
    op->setHidden(checked);
    op->getOwnerPage()->updateLinks();

    Q_EMIT onOperatorChanged(eNULL);
}

void eParameterFrame::_onNameChanged(const QString &text)
{
    eIOperator *op = eDemoData::findOperator(m_opId);
    eASSERT(op != eNULL);
    op->setUserName(text.toAscii().constData());

    Q_EMIT onOperatorChanged(eNULL);
}

void eParameterFrame::_onParameterChanged(const eParameter &param)
{
    Q_EMIT onOperatorChanged(&param);
}