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

#include <QtGui/QAction>
#include <QtGui/QMessageBox>

#include "instrumentlist.hpp"

#include "../../eshared/eshared.hpp"

eInstrumentList::eInstrumentList(QWidget *parent) : QTableWidget(parent)
{
}

void eInstrumentList::initRows()
{
    setRowCount(TF_MAX_INPUTS);
    
    for(eU32 i=0; i<TF_MAX_INPUTS; i++)
    {
        QTableWidgetItem *twi = new QTableWidgetItem("<empty>");
        eASSERT(twi != eNULL);
        twi->setFlags(twi->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsEnabled);

        setItem(i, 0, twi);
        setRowHeight(i, 20);
    }

    for(eU32 i=0; i<TF_MAX_INPUTS; i++)
    {
        QTableWidgetItem *twi = new QTableWidgetItem(QString::number(i, 16).toUpper());
        eASSERT(twi != eNULL);
        setVerticalHeaderItem(i, twi);
    }
}

void eInstrumentList::saveToXml(QDomElement &node) const
{
    QDomDocument xml = node.ownerDocument();
    eASSERT(!xml.isNull());
    QDomElement instrumentsEl = xml.createElement("instruments");
    node.appendChild(instrumentsEl);
    
    for (eU32 i=0; i<TF_MAX_INPUTS; i++)
    {
        QTableWidgetItem *twi = item(i, 0);
        eASSERT(twi != eNULL);

        tfInstrument *tf = eDemo::getSynth().getInstrument(i);

        if (tf)
        {
            QDomElement instrEl = xml.createElement("instrument");
            instrumentsEl.appendChild(instrEl);
            instrEl.setAttribute("name", twi->text());
            instrEl.setAttribute("id", i);

            QDomElement paramsEl = xml.createElement("parameters");
            instrEl.appendChild(paramsEl);

            for (eU32 j=0; j<TF_PARAM_COUNT; j++)
            {
                QDomElement paramEl = xml.createElement("param");
                paramsEl.appendChild(paramEl);

				const eF32 value = tf->getParam(j);
                
				//const eU32 ivalue = eFtoL(eRound(value*99));

                paramEl.setAttribute("name", TF_NAMES[j]);
                paramEl.setAttribute("value", value);
            }
        }
    }
}

void eInstrumentList::loadFromXml(const QDomElement &node)
{
    QDomElement xmlItem = node.firstChildElement("instrument");
    tfPlayer &player = eDemo::getSynth();

    while (!xmlItem.isNull())
    {
        const QString name = xmlItem.attribute("name"); 
        const eU32 id = xmlItem.attribute("id").toInt(); 

        player.addInstrument(id);

        QTableWidgetItem *twi = item(id, 0);
        eASSERT(twi != eNULL);
        twi->setText(name);
        twi->setFlags(twi->flags() | Qt::ItemIsEnabled);

        tfInstrument *tf = player.getInstrument(id);
        eASSERT(tf != eNULL);

        QDomElement xmlItemParam = xmlItem.firstChildElement("parameters");
        xmlItemParam = xmlItemParam.firstChildElement("param");

        while (!xmlItemParam.isNull())
        {
            const QString param = xmlItemParam.attribute("name");
            const QString svalue = xmlItemParam.attribute("value");
			//const eU32 ivalue = svalue.toInt();
            //const eF32 value = (eF32)ivalue/99.0f; 
            const eF32 value = svalue.toFloat();

            for (eU32 i=0; i<TF_PARAM_COUNT; i++)
            {
                if (param == TF_NAMES[i])
                {
                    //if (i == TF_FORMANT_WET) __asm int 3;

                    tf->setParam(i, value);
                    break;
                }
            }

            xmlItemParam = xmlItemParam.nextSiblingElement("param");
        }

        xmlItem = xmlItem.nextSiblingElement("instrument");
    }
}