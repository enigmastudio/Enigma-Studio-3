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

#include "../eshared.hpp"

#define USE_EQUIVALENT_OP_SEARCH

#ifdef eDEBUG
#include <windows.h>
#include <stdio.h>
#endif


// Initialize static members.
#ifdef eEDITOR
eOpPagePtrArray                 eDemoData::m_pages;
#else
eIOperator *                    eDemoData::m_mainDemoOp;
#endif
tfSongPtrArray                  eDemoData::m_songs;

#ifdef ePLAYER
eDemoData::VirtualFilePtrArray  eDemoData::m_virtFiles;
#else
void eDemoData::free()
{
    clearPages();
    clearSongs();
}
#endif

#define ESCRIPT_CMD_TYPE eU32 
#define ESCRIPT_CMD__OP_DONE        0
#define ESCRIPT_CMD__INPUTS_DONE    1
#define ESCRIPT_CMD__OP_NOEXISTS    2
#define ESCRIPT_CMDS_COUNT 3
eArray<eU32>    opTypeHashIDs;

#ifdef ePLAYER

eIOperatorPtrArray global_ops;

void eDemoData::load(eDemoScript &script)
{
    // Demo-Op is always first operator typ, offsetted with ESCRIPT_CMDS_COUNT
    m_mainDemoOp = _loadOperator(script, ESCRIPT_CMDS_COUNT);

    for(eU32 storePass = 0; storePass < 2; storePass++) {
        for(eU32 t = 0; t < TOTAL_OP_TYPE_COUNT; t++) {
            eBool moreParams = eTRUE;
            eS32 paramNr = 0;
            while(moreParams) {
                for(eU32 o = 0; o < global_ops.size(); o++) {
                    eIOperator& op = *global_ops[o];
                    if(t == op.m_metaOpID) {
        #ifdef eDEBUG
            char debbuf[1000];
            sprintf(debbuf, "Operator\n");
            OutputDebugString(debbuf);
        #endif
                        if(paramNr >= op.getParameterCount()) {
                            moreParams = eFALSE;
                            break;
                        }
                        op.getParameter(paramNr).load(script, storePass);
                    }
                }
                paramNr++;
            };
        }
    }
}

void eDemoData::addVirtualFile(const eString &fileName, const eByteArray &block)
{
    for (eU32 i=0; i<m_virtFiles.size(); i++)
    {
        if (m_virtFiles[i]->fileName == fileName)
        {
            // File already exists.
            return;
        }
    }

    // Add new virtual file.
    VirtualFile *vf = new VirtualFile;
    eASSERT(vf != eNULL);
    vf->fileName = fileName;
    vf->data = block;

    m_virtFiles.append(vf);
}

eBool eDemoData::removeVirtualFile(const eString &fileName)
{
    for (eU32 i=0; i<m_virtFiles.size(); i++)
    {
        VirtualFile *vf = m_virtFiles[i];
        eASSERT(vf != eNULL);

        if (vf->fileName == fileName)
        {
            eSAFE_DELETE(vf);
            m_virtFiles.removeAt(i);

            return eTRUE;
        }
    }

    return eFALSE;
}

const eByteArray * eDemoData::getVirtualFile(const eString &fileName)
{
    for (eU32 i=0; i<m_virtFiles.size(); i++)
    {
        VirtualFile *vf = m_virtFiles[i];
        eASSERT(vf != eNULL);

        if (vf->fileName == fileName)
        {
            return &vf->data;
        }
    }

    return eNULL;
}
#endif


#ifdef eEDITOR

eArray<eDemoData::tStatRecord> eDemoData::stats;
eArray<const eString*>    opClassNames;
eArray<const eString*>    opClassFiles;
eHashMap<eIOperator const *, eBool>                 opStoredHm;
eHashMap<eIOperator const *, eID>                   opIdHm;
eHashMap<const eIOperator *, const eIOperator *>    opEquivalentOp;
eHashMap<const eIOperator *, const eIOperator *>    opUniqueOps;

const eIOperator * eDemoData::_resolveOperator(const eIOperator *op)
{
    if (!op)
    {
        return eNULL;
    }

    while (op->getRealType() == "Misc : Load" || op->getRealType() == "Misc : Store" || op->getRealType() == "Misc : Nop")
    {
        if (op->getRealType() == "Misc : Load")
        {
            op = eDemoData::findOperator(op->getParameter(0).getValue().linkedOpId);
            eASSERT(op != eNULL);
        }
        else if (op->getRealType() == "Misc : Store" || op->getRealType() == "Misc : Nop")
        {
            op = op->getInputOperator(0);
            eASSERT(op != eNULL);
        }
    }

    return op;
}


void eDemoData::_preprocessOpTree(const eIOperator *op) {
    // check if this op has already been processed
    if(opEquivalentOp.exists(op))
        return;

    // create op type id if necessary
    eU32 opTypeHashID = eOpFactory::get().toHashID(op->getType());
    if(-1 == opTypeHashIDs.exists(opTypeHashID)) {
        tStatRecord s;
        s.name = &op->getType();
        s.cnt = 0;
        s.accsize = 0;
        opClassNames.append(&op->getMetaInfos().classNameString);
        opClassFiles.append(&op->getMetaInfos().sourceFileName);
        stats.append(s);
        opTypeHashIDs.append(opTypeHashID);
    }

    // try to find an equivalent op for inputs and params
    for (eU32 i=0; i<op->getInputCount(); i++) {
        const eIOperator *inputOp = _resolveOperator(op->getInputOperator(i));
        _preprocessOpTree(inputOp);
    }
    for (eU32 i=0; i<op->getParameterCount(); i++) {
        const eParameter& param = op->getParameter(i);
        if(param.getType() == eParameter::TYPE_LINK) {
            const eIOperator *linkOp = _resolveOperator(findOperator(param.getValue().linkedOpId));
            if(linkOp)
                _preprocessOpTree(linkOp);
        }
        if(param.isAnimated()) {
            const eIOperator *linkOp = _resolveOperator(findOperator(param.m_animPathOpId));
            if(linkOp)
                _preprocessOpTree(linkOp);
        }
    }

    // try to find an equivalent op for this op
    const eIOperator *eqOp = op;
    for(eU32 o = 0; o < opUniqueOps.getCount(); o++) {
        const eIOperator *testOp = opUniqueOps.getAt(o);
        
        // ensure type and common cardinalities are identical
        if((eStrCompare(op->getType(), testOp->getType()) != 0) ||
           (op->getParameterCount() != testOp->getParameterCount()) ||
           (op->getInputCount() != testOp->getInputCount()))
           continue;

        bool notEqual = false;
        // compare inputs
        for (eU32 i=0; i<op->getInputCount(); i++) {
            const eIOperator *inputOp1 = _resolveOperator(op->getInputOperator(i));
            const eIOperator *inputOp2 = _resolveOperator(testOp->getInputOperator(i));
            if(opEquivalentOp[inputOp1] != opEquivalentOp[inputOp2]) {
                notEqual = true;
                break;
            }
        }
        if(notEqual)
            continue;

        // compare parameters
        for (eU32 i=0; i<op->getParameterCount(); i++) {
            const eParameter& param1 = op->getParameter(i);
            const eParameter& param2 = testOp->getParameter(i);
            if((param1.getType() != param2.getType()) ||
                (param1.isAnimated() != param2.isAnimated())) {
                notEqual = true;
                break;
            }
            if(param1.getType() == eParameter::TYPE_LINK) {
                const eIOperator *linkOp1 = _resolveOperator(findOperator(param1.getValue().linkedOpId));
                const eIOperator *linkOp2 = _resolveOperator(findOperator(param2.getValue().linkedOpId));
                if(opEquivalentOp[linkOp1] != opEquivalentOp[linkOp2]) {
                    notEqual = true;
                    break;
                }
            } else {
                // check raw params
                if(param1.isAnimated()) {
                    const eIOperator *linkOp1 = _resolveOperator(findOperator(param1.m_animPathOpId));
                    const eIOperator *linkOp2 = _resolveOperator(findOperator(param2.m_animPathOpId));
                    if( (param1.getAnimationChannel() != param2.getAnimationChannel()) ||
                        (param1.getAnimationTimeOffset() != param2.getAnimationTimeOffset()) ||
                        (param1.getAnimationTimeScale() != param2.getAnimationTimeScale()) ||
                        (opEquivalentOp[linkOp1] != opEquivalentOp[linkOp2])) {
                        notEqual = true;
                        break;
                    }
                } else {
                    // plain value parameter
                    if(param1.isTextType()) {
                        if(eStrCompare(param1.m_value.string, param2.m_value.string) != 0) {
                            notEqual = true;
                            break;
                        }
                    } else {
                        if(!eMemEqual(&param1.m_value, &param2.m_value, eParameter::M_RELEVANT_PARAMETER_LENGTH[param1.m_type])) {
                            notEqual = true;
                            break;
                        }
                    }
                    // check bytecode
                    if((param1.m_value.byteCode.size() != param2.m_value.byteCode.size()) ||
                        ((param1.m_value.byteCode.size() != 0) && (!eMemEqual(param1.m_value.byteCode.m_data, param2.m_value.byteCode.m_data, param1.m_value.byteCode.size())))) {
                            notEqual = true;
                            break;
                    }
                }
            }
        }
#ifdef USE_EQUIVALENT_OP_SEARCH
        if(notEqual)
#endif
            continue;
        // all parameters are equal
        eASSERT(eqOp == op);
        eqOp = testOp;
    } 

    if(eqOp == op) {
        // operator is unique
        opUniqueOps.insert(op, op);
    };
    opEquivalentOp.insert(op, eqOp);
}

void eDemoData::_storeOperator(eDemoScript &script, const eIOperator *op, eID &idCounter, eArray<const eIOperator*>& ops)
{
    eASSERT(idCounter != eNOID);
    eASSERT(op->getType() != "Misc : Nop");
    eASSERT(op->getType() != "Misc : Load");
    eASSERT(op->getType() != "Misc : Store");

    opIdHm[op] = idCounter;
    opStoredHm[op] = eTRUE;

    eU32 statIdx = opTypeHashIDs.exists(eOpFactory::get().toHashID(op->getType()));
    stats[statIdx].cnt++;

    ops.append(op);

    // inputs
    for (eU32 i=0; i<op->getInputCount(); i++)
    {
        const eIOperator *inputOp = opEquivalentOp[_resolveOperator(op->getInputOperator(i))];

		if (opStoredHm.exists(inputOp))
        {
            script << (ESCRIPT_CMD_TYPE)ESCRIPT_CMD__OP_DONE;
            eASSERT(opIdHm.exists(inputOp) == eTRUE);
            eID id = opIdHm[inputOp];
            script << (eU16)id;

            stats[statIdx].accsize += sizeof(ESCRIPT_CMD_TYPE) + sizeof(eU16);
        }
        else
        {
            eU32 opTypeHashID = eOpFactory::get().toHashID(inputOp->getType());
            script << (ESCRIPT_CMD_TYPE)(opTypeHashIDs.exists(opTypeHashID) + ESCRIPT_CMDS_COUNT);
            _storeOperator(script, inputOp, ++idCounter, ops);

            stats[statIdx].accsize += sizeof(ESCRIPT_CMD_TYPE);
        }
    }

    script << (ESCRIPT_CMD_TYPE)ESCRIPT_CMD__INPUTS_DONE;
    stats[statIdx].accsize += sizeof(ESCRIPT_CMD_TYPE);

    // linked ops
    for (eU32 i=0; i<op->getParameterCount(); i++)
    {
        if (op->getParameter(i).getType() == eParameter::TYPE_LINK)
        {
            const eIOperator *linkOpRaw = findOperator(op->getParameter(i).getValue().linkedOpId);

            if (linkOpRaw)
            {
                const eIOperator *linkOp = opEquivalentOp[_resolveOperator(linkOpRaw)];
                eASSERT(linkOp != eNULL);

                if (opStoredHm.exists(linkOp))
                {
                    eASSERT(opIdHm.exists(linkOp) == eTRUE);
                    script << (ESCRIPT_CMD_TYPE)ESCRIPT_CMD__OP_DONE;
                    eID id = opIdHm[linkOp];
                    script << (eU16)id;

                    stats[statIdx].accsize += sizeof(ESCRIPT_CMD_TYPE) + sizeof(eU16);
                }
                else
                {
                    eU32 opTypeHashID = eOpFactory::get().toHashID(linkOp->getType());
                    script << (ESCRIPT_CMD_TYPE)(opTypeHashIDs.exists(opTypeHashID) + ESCRIPT_CMDS_COUNT);
                    _storeOperator(script, linkOp, ++idCounter, ops);

                    stats[statIdx].accsize += sizeof(ESCRIPT_CMD_TYPE);
                }
            }
            else
            {
                script << (ESCRIPT_CMD_TYPE)ESCRIPT_CMD__OP_NOEXISTS;
                stats[statIdx].accsize += sizeof(ESCRIPT_CMD_TYPE);
            }
        }
	}

	// animated params
    eU32 paramNr = 0;
	for (eU32 i=0; i<op->getParameterCount(); i++)
    {
        if(op->getParameter(i).getType() == eParameter::TYPE_LABEL)
            continue;
        if (op->getParameter(i).getType() != eParameter::TYPE_LINK)
        {
            if (op->getParameter(i).isAnimated())
            {
                const eIOperator *animOp = findOperator(op->getParameter(i).getAnimationPathOpId());

                if (animOp)
                {
					script << (eU8)(paramNr+1);
                    animOp = opEquivalentOp[_resolveOperator(animOp)];

                    if (opStoredHm.exists(animOp))
                    {
                        eASSERT(opIdHm.exists(animOp) == eTRUE);
                        script << (ESCRIPT_CMD_TYPE)ESCRIPT_CMD__OP_DONE;
                        eID id = opIdHm[animOp];
                        script << (eU16)id;

                        stats[statIdx].accsize += sizeof(ESCRIPT_CMD_TYPE) + sizeof(eU16);
                    }
                    else
                    {
                        eU32 opTypeHashID = eOpFactory::get().toHashID(animOp->getType());
                        script << (ESCRIPT_CMD_TYPE)(opTypeHashIDs.exists(opTypeHashID) + ESCRIPT_CMDS_COUNT);
                        _storeOperator(script, animOp, ++idCounter, ops);

                        stats[statIdx].accsize += sizeof(ESCRIPT_CMD_TYPE);
                    }
                }
            }
		}
        paramNr++;
	}

	script << (eU8)0; // end flag for animated parameters loading

    stats[statIdx].accsize += sizeof(eU8);

}

eBool sortByAccSize(const eDemoData::tStatRecord& rec0, const eDemoData::tStatRecord& rec1)
{
    return rec0.accsize < rec1.accsize;
}

void eDemoData::store(eDemoScript &script, const eIDemoOp *demoOp)
{
    eASSERT(demoOp != eNULL);

    opStoredHm.clear();
    opIdHm.clear();

    opEquivalentOp.clear();
    opUniqueOps.clear();
    opTypeHashIDs.clear();
    opClassNames.clear();
    opClassFiles.clear();
    stats.clear();
    stats.reserve(1000);
    _preprocessOpTree(demoOp);

    eArray<const eIOperator*> ops;

    eID idCounter = 1;
    _storeOperator(script, demoOp, idCounter, ops);

    // now store operators
    for(eU32 storePass = 0; storePass < 2; storePass++) {
        for(eU32 t = 0; t < opTypeHashIDs.size(); t++) {
            eS32 parNr = 0;
            while(parNr != -1) 
            {
                for(eU32 o = 0; o < ops.size(); o++) {
                    const eIOperator* op = ops[o];
                    if(t == opTypeHashIDs.exists(eOpFactory::get().toHashID(op->getType()))) {
                        // Add this operator to the list of used operators.
                        script.addUsedOp(op);

                        if(parNr >= (eS32)op->getParameterCount()) {
                            parNr = -2;
                            break;
                        }

                        eU32 statIdx = opTypeHashIDs.exists(eOpFactory::get().toHashID(op->getType()));
                        eU32 lenBefore = script.m_byteLength;

                        // copy original parameters
                        eArray<eParameter::Value> valCopies;
                        valCopies.reserve(op->getParameterCount());
                        for(eU32 i = 0; i < op->getParameterCount(); i++)
                            valCopies.append(op->getParameter(i).m_value);
                        // call optimizer
                        ((eIOperator *)op)->optimizeForExport();

/*
        #ifdef eDEBUG
            char debbuf[1000];
            sprintf(debbuf, "Operator\n");
            OutputDebugString(debbuf);
        #endif
*/
                        if(op->m_params[parNr]->getType() != eParameter::TYPE_LABEL) {
                            // save actual parameters in second pass
                            op->m_params[parNr]->store(script, storePass);
                        }
    //                    op->store(script);

                        // restore original parameters
                        for(eU32 i = 0; i < op->getParameterCount(); i++)
                            ((eIOperator *)op)->getParameter(i).m_value = valCopies[i];

                        stats[statIdx].accsize += script.m_byteLength - lenBefore;
                    }
                }
                parNr++;
            }
        }
    }
    stats.sort(sortByAccSize);
}

eString eDemoData::getUsedOpCpp()
{
    eString result;
    for(eU32 i = 0; i < opClassNames.size(); i++) {
        result += *opClassNames[i];
        result += ";";
        result += *opClassFiles[i];
        result += "\r\n";
    }
    return result;
}

#endif


#ifdef eEDITOR
eOperatorPage * eDemoData::addPage(eID pageId)
{
    eOperatorPage *page = new eOperatorPage(pageId);
    eASSERT(page != eNULL);
    m_pages.append(page);
    return page;
}

eBool eDemoData::removePage(eID pageId)
{
    for (eU32 i=0; i<m_pages.size(); i++)
    {
        if (m_pages[i]->getId() == pageId)
        {
            eSAFE_DELETE(m_pages[i]);
            m_pages.removeAt(i);
            return eTRUE;
        }
    }

    return eFALSE;
}

void eDemoData::clearPages()
{
    for (eU32 i=0; i<m_pages.size(); i++)
    {
        eSAFE_DELETE(m_pages[i]);
    }

    m_pages.clear();
}

void eDemoData::updateAllPageLinks()
{
    for (eU32 i=0; i<m_pages.size(); i++)
    {
        m_pages[i]->updateLinks();
    }
}

eBool eDemoData::existsPage(eID pageId)
{
    return (getPageById(pageId) != eNULL);
}

eU32 eDemoData::getPageCount()
{
    return m_pages.size();
}

eOperatorPage * eDemoData::getPageByIndex(eU32 index)
{
    eASSERT(index < m_pages.size());
    return m_pages[index];
}

eOperatorPage * eDemoData::getPageById(eID pageId)
{
    for (eU32 i=0; i<m_pages.size(); i++)
    {
        if (m_pages[i]->getId() == pageId)
        {
            return m_pages[i];
        }
    }

    return eNULL;
}

eU32 eDemoData::getTotalOpCount()
{
    eU32 count = 0;

    for (eU32 i=0; i<m_pages.size(); i++)
    {
        count += m_pages[i]->getOperatorCount();
    }

    return count;
}

eIOperator * eDemoData::findOperator(eID opId)
{
    ePROFILER_ZONE("Find operator");

    for (eU32 i=0; i<m_pages.size(); i++)
    {
        eIOperator *op = m_pages[i]->getOperatorById(opId);

        if (op)
        {
            return op;
        }
    }

    return eNULL;
}
#else

eIOperator * eDemoData::findOperator(eID opId)
{
    for(eU32 i = 0; i < global_ops.size(); i++)
        if(global_ops[i]->getId() == opId)
            return global_ops[i];
    return eNULL;
}
#endif

eBool eDemoData::existsOperator(eID opId)
{
    return (findOperator(opId) != eNULL);
}

#ifdef ePLAYER
eIDemoOp * eDemoData::getMainDemoOperator()
{
    return (eIDemoOp *)m_mainDemoOp;
}
#endif

tfSong * eDemoData::newSong()
{
    m_songs.append(new tfSong);
    return m_songs[m_songs.size()-1];
}

void eDemoData::removeSong(eID songId)
{
    for (eU32 i=0; i<m_songs.size(); i++)
    {
        if (m_songs[i]->getId() == songId)
        {
            eSAFE_DELETE(m_songs[i]);
            m_songs.removeAt(i);
            break;
        }
    }
}

void eDemoData::clearSongs()
{
    for (eU32 i=0; i<m_songs.size(); i++)
    {
        eSAFE_DELETE(m_songs[i]);
    }

    m_songs.clear();
}

tfSong * eDemoData::getSongByIndex(eU32 index)
{
    if (index >= m_songs.size())
        return eNULL;

    return m_songs[index];
}

tfSong * eDemoData::getSongById(eID songId)
{
    for (eU32 i=0; i<m_songs.size(); i++)
    {
        if (m_songs[i]->getId() == songId)
        {
            return m_songs[i];
        }
    }

    return eNULL;
}

eU32 eDemoData::getSongCount()
{
    return m_songs.size();
}

#ifdef ePLAYER
eIOperator * eDemoData::_loadOperator(eDemoScript &script, eU32 opTypeHashIdIndex)
{
    eIOperator *op = SCRIPT_OP_CREATOR::createOp(opTypeHashIdIndex - ESCRIPT_CMDS_COUNT);
    op->m_id = global_ops.size()+1;
    global_ops.append(op);

    // load inputs
    while (eTRUE)
    {
        ESCRIPT_CMD_TYPE command;
        script >> command;

        if (command == ESCRIPT_CMD__INPUTS_DONE)
        {
            break;
        }
        else if (command == ESCRIPT_CMD__OP_DONE)
        {
            eU16 id;
            script >> id;
            eIOperator *inputOp = findOperator(id);
            eASSERT(inputOp != eNULL);
            op->m_inputOps.append(inputOp);
            inputOp->m_outputOps.append(op);
        }
        else
        {
            eIOperator *inputOp = _loadOperator(script, command);
            op->m_inputOps.append(inputOp);
            inputOp->m_outputOps.append(op);
        }
    }
    
    // load params
    for (eU32 i=0; i<op->getParameterCount(); i++)
    {
        if (op->getParameter(i).getType() == eParameter::TYPE_LINK)
        {
            ESCRIPT_CMD_TYPE paramCommand;
            script >> paramCommand;

            if (paramCommand != ESCRIPT_CMD__OP_NOEXISTS)
            {
                if (paramCommand == ESCRIPT_CMD__OP_DONE)
                {
                    eU16 id;
                    script >> id;
                    op->getParameter(i).getValue().linkedOpId = id;
                    findOperator(id)->m_linkingOps.append(op->getId());
                }
                else
                {
                    eIOperator *paramOp = _loadOperator(script, paramCommand);
                    eASSERT(paramOp != eNULL);
                    op->getParameter(i).getValue().linkedOpId = paramOp->getId();
                    paramOp->m_linkingOps.append(op->getId());
                }
            }
        }
	}

    // load animated parameters
	while (eTRUE)
    {
        eU8 paramNr;
		script >> paramNr;

		if (paramNr == 0)
			break;
		paramNr--;

        ESCRIPT_CMD_TYPE animCommand;
        script >> animCommand;
        op->getParameter(paramNr).m_animated = eTRUE;

        if (animCommand == ESCRIPT_CMD__OP_DONE)
        {
            eU16 id;
            script >> id;
            op->getParameter(paramNr).m_animPathOpId = id;
            findOperator(id)->m_linkingOps.append(op->getId());
        }
        else
        {
            eIOperator *paramOp = _loadOperator(script, animCommand);
            op->getParameter(paramNr).m_animPathOpId = paramOp->getId();
            paramOp->m_linkingOps.append(op->getId());
        }
	}

    return op;
}
#else
#endif