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

#ifndef OP_MACROS_HPP
#define OP_MACROS_HPP

#ifdef eEDITOR
#define OP_DEFINE(className, opID, baseClass, name, category, catID, color, shortcut, minInput, maxInput, allowedInput)      \
    static eIOperator::MetaInfos className##MetaInfos =                                                         \
    {                                                                                                           \
        #category, name, color, shortcut, minInput, maxInput, allowedInput, #className, __FILE__, eString(#category)+" : "+name, eNULL  \
    };                                                                                                          \
                                                                                                                \
    class className : public baseClass                                                                          \
    {																											\
    public:                                                                                                     \
        className()                                                                                             \
        {                                                                                                       \
            m_metaInfos = &className##MetaInfos;                                                                \
                                                                                                                \
            setWidth(eClamp<eU32>(1, (eU32)maxInput, 3)*4);                                                     \
            _initialize();                                                                                      \
        }                                                                                                       \
                                                                                                                \
        virtual ~className()                                                                                    \
        {                                                                                                       \
            _deinitialize();                                                                                    \
        }

#define TEST_OP_TYPE(op, typeString_Editor, opID_player)                                                       \
    (op->getType() == typeString_Editor)
#define TEST_CATEGORY(op, catString_Editor, catID_player)                                                       \
    (op->getCategory() == catString_Editor)

#else
#define OP_DEFINE(className, opID, baseClass, name, category, catID, color, shortcut, minInput, maxInput, allowedInput)      \
                                                                                                                \
    class className : public baseClass                                                                          \
    {																											\
    public:                                                                                                     \
        className()                                                                                             \
        {                                                                                                       \
            m_metaOpID = opID;                                                                                  \
            m_metaCategoryID = catID;                                                                           \
            ePtr func = eNULL;                                                                                  \
            __asm                                                                                               \
            {                                                                                                   \
                __asm mov eax, dword ptr [className::execute]                                                   \
                __asm mov dword ptr [func], eax                                                                 \
            }                                                                                                   \
            m_metaExecFunc = func;                                                                              \
            _initialize();                                                                                      \
        }                                                                                                       

#define TEST_OP_TYPE(op, typeString_Editor, opID_player)                                                       \
    (op->m_metaOpID == opID_player)
#define TEST_CATEGORY(op, catString_Editor, catID_player)                                                       \
    (op->m_metaCategoryID == catID_player)

#endif

#define OP_DEFINE_BITMAP(className, classNameString, name, shortcut, minInput, maxInput, allowedInput)                           \
    OP_DEFINE(className, classNameString, eIBitmapOp, name, Bitmap, Bitmap_CID, eColor(185, 230, 115), shortcut, minInput, maxInput, allowedInput)

#define OP_DEFINE_EFFECT(className, classNameString, name, shortcut, minInput, maxInput, allowedInput)                           \
    OP_DEFINE(className, classNameString, eIEffectOp, name, Effect, Effect_CID, eColor(116, 255, 255), shortcut, minInput, maxInput, allowedInput)

#define OP_DEFINE_MESH(className, classNameString, name, shortcut, minInput, maxInput, allowedInput)                             \
    OP_DEFINE(className, classNameString, eIMeshOp, name, Mesh, Mesh_CID, eColor(116, 160, 173), shortcut, minInput, maxInput, allowedInput)

#define OP_DEFINE_SEQ(className, classNameString, name, shortcut, minInput, maxInput, allowedInput)                              \
    OP_DEFINE(className, classNameString, eISequencerOp, name, Sequencer, Sequencer_CID, eColor(255, 128, 0), shortcut, minInput, maxInput, allowedInput)

#define OP_DEFINE_MODEL(className, classNameString, name, shortcut, minInput, maxInput, allowedInput)                            \
    OP_DEFINE(className, classNameString, eIModelOp, name, Model, Model_CID, eColor(215, 215, 0), shortcut, minInput, maxInput, allowedInput)

#define OP_DEFINE_PATH(className, classNameString, name, shortcut, minInput, maxInput, allowedInput)                             \
    OP_DEFINE(className, classNameString, eIPathOp, name, Path, Path_CID, eColor(0, 128, 255), shortcut, minInput, maxInput, allowedInput)

#ifdef eEDITOR
#define OP_END(className)                                                                                       \
        static const struct RegisterExecFunc                                                                    \
        {                                                                                                       \
            RegisterExecFunc()                                                                                  \
            {                                                                                                   \
                ePtr func = eNULL;                                                                              \
                                                                                                                \
                __asm                                                                                           \
                {                                                                                               \
                    __asm mov eax, dword ptr [className::execute]                                               \
                    __asm mov dword ptr [func], eax                                                             \
                }                                                                                               \
                                                                                                                \
                className##MetaInfos.execFunc = func;                                                           \
                eOpMetaInfoManager::addMetaInfos(&className##MetaInfos);                                        \
            }                                                                                                   \
        }                                                                                                       \
        regExecFunc;                                                                                            \
    };                                                                                                          \
                                                                                                                \
    const className::RegisterExecFunc className::regExecFunc;                                                   \
    eFACTORY_REGISTER(eOpFactory, className, className##MetaInfos.type);
#else
#define OP_END(className)                                                                                       \
    };                                                                                                          
#endif

#define OP_EXEC                                                                                                 \
   public:                                                                                                      \
       void execute

#define OP_INIT                                                                                                 \
   private:																										\
       void _initialize


#define OP_DEINIT                                                                                               \
    private:                                                                                                    \
        void _deinitialize

#define OP_OPTIMIZE_FOR_EXPORT                                                                                  \
   public:																										\
       virtual void optimizeForExport

#define OP_INTERACT                                                                                             \
    public:                                                                                                     \
    	void doEditorInteraction

#define OP_VAR(var)                                                                                             \
    private:                                                                                                    \
        var


#ifdef eEDITOR

#define eOP_PARAM_ADD_INT(name, min, max, value)                                                                \
    addParameter(eParameter::TYPE_INT, name, (eF32)min, (eF32)max, value);

#define eOP_PARAM_ADD_FLOAT(name, min, max, value)                                                              \
    addParameter(eParameter::TYPE_FLOAT, name, min, max, value);

#define eOP_PARAM_ADD_ENUM(name, items, index)                                                                  \
    addParameter(eParameter::TYPE_ENUM, name, items, index);

#define eOP_PARAM_ADD_FLAGS(name, items, index)                                                                 \
    addParameter(eParameter::TYPE_FLAGS, name, items, index);

#define eOP_PARAM_ADD_BOOL(name, state)                                                                         \
    addParameter(eParameter::TYPE_BOOL, name, 0, 0, eVector4(state, 0, 0, 0));

#define eOP_PARAM_ADD_STRING(name, string)                                                                      \
    addParameter(eParameter::TYPE_STRING, name, string);

#define eOP_PARAM_ADD_TEXT(name, text)                                                                          \
    addParameter(eParameter::TYPE_TEXT, name, text);

#define eOP_PARAM_ADD_TSHADERCODE(name, string)                                                                  \
    addParameter(eParameter::TYPE_TSHADERCODE, name, string);

#define eOP_PARAM_ADD_FILE(name, file)                                                                          \
    addParameter(eParameter::TYPE_FILE, name, file);

#define eOP_PARAM_ADD_FXY(name, min, max, x, y)                                                                 \
    addParameter(eParameter::TYPE_FXY, name, min, max, eVector4(x, y, 0.0f, 0.0f));

#define eOP_PARAM_ADD_FXYZ(name, min, max, x, y, z)                                                             \
    addParameter(eParameter::TYPE_FXYZ, name, min, max, eVector4(x, y, z, 0.0f));

#define eOP_PARAM_ADD_FXYZW(name, min, max, x, y, z, w)                                                         \
    addParameter(eParameter::TYPE_FXYZW, name, min, max, eVector4(x, y, z, w));

#define eOP_PARAM_ADD_RGB(name, red, green, blue)                                                               \
    addParameter(eParameter::TYPE_RGB, name, 0.0f, 1.0f, eVector4(red, green, blue, 0.0f));

#define eOP_PARAM_ADD_RGBA(name, red, green, blue, alpha)                                                       \
    addParameter(eParameter::TYPE_RGBA, name, 0.0f, 1.0f, eVector4(red, green, blue, alpha));

#define eOP_PARAM_ADD_IXY(name, min, max, x, y)                                                                 \
    addParameter(eParameter::TYPE_IXY, name, min, max, eVector4(x, y, 0, 0));

#define eOP_PARAM_ADD_IXYZ(name, min, max, x, y, z)                                                             \
    addParameter(eParameter::TYPE_IXYZ, name, min, max, eVector4(x, y, z, 0));

#define eOP_PARAM_ADD_IXYXY(name, min, max, x0, y0, x1, y1)                                                     \
    addParameter(eParameter::TYPE_IXYXY, name, min, max, eVector4(x0, y0, x1, y1));

#define eOP_PARAM_ADD_LABEL(name, caption)                                                                      \
    addParameter(eParameter::TYPE_LABEL, name, caption);

#define eOP_PARAM_ADD_LINK(name, allowedLinks)                                                                  \
    addParameter(eParameter::TYPE_LINK, name, 0, 0, eVector4(0)).setAllowedLinks(allowedLinks);

#define eOP_PARAM_ADD_SYNTH(name)                                                                               \
    addParameter(eParameter::TYPE_SYNTH, name, 0.0f, 255.0f, 0.0f);

#elif ePLAYER

#define eOP_PARAM_ADD_INT(name, min, max, value)                                                                \
    addParameter(eParameter::TYPE_INT);

#define eOP_PARAM_ADD_FLOAT(name, min, max, value)                                                              \
    addParameter(eParameter::TYPE_FLOAT);

#define eOP_PARAM_ADD_ENUM(name, items, index)                                                                  \
    addParameter(eParameter::TYPE_ENUM);

#define eOP_PARAM_ADD_FLAGS(name, items, index)                                                                 \
    addParameter(eParameter::TYPE_FLAGS);

#define eOP_PARAM_ADD_BOOL(name, state)                                                                         \
    addParameter(eParameter::TYPE_BOOL);

#define eOP_PARAM_ADD_STRING(name, string)                                                                      \
    addParameter(eParameter::TYPE_STRING);

#define eOP_PARAM_ADD_TEXT(name, text)                                                                          \
    addParameter(eParameter::TYPE_TEXT);

#define eOP_PARAM_ADD_TSHADERCODE(name, string)                                                                 \
    addParameter(eParameter::TYPE_TSHADERCODE);

#define eOP_PARAM_ADD_FILE(name, file)                                                                          \
    addParameter(eParameter::TYPE_FILE);

#define eOP_PARAM_ADD_FXY(name, min, max, x, y)                                                                 \
    addParameter(eParameter::TYPE_FXY);

#define eOP_PARAM_ADD_FXYZ(name, min, max, x, y, z)                                                             \
    addParameter(eParameter::TYPE_FXYZ);

#define eOP_PARAM_ADD_FXYZW(name, min, max, x, y, z, w)                                                         \
    addParameter(eParameter::TYPE_FXYZW);

#define eOP_PARAM_ADD_RGB(name, r, g, b)                                                                        \
    addParameter(eParameter::TYPE_RGB);

#define eOP_PARAM_ADD_RGBA(name, r, g, b, a)                                                                    \
    addParameter(eParameter::TYPE_RGBA);

#define eOP_PARAM_ADD_IXY(name, min, max, x, y)                                                                 \
    addParameter(eParameter::TYPE_IXY);

#define eOP_PARAM_ADD_IXYZ(name, min, max, x, y, z)                                                             \
    addParameter(eParameter::TYPE_IXYZ);

#define eOP_PARAM_ADD_IXYXY(name, min, max, x0, y0, x1, y1)                                                     \
    addParameter(eParameter::TYPE_IXYXY);

#define eOP_PARAM_ADD_LABEL(name, caption)

#define eOP_PARAM_ADD_LINK(name, allowed)                                                                       \
    addParameter(eParameter::TYPE_LINK);

#define eOP_PARAM_ADD_SYNTH(name)                                                                               \
    addParameter(eParameter::TYPE_SYNTH);

#endif

#endif // OP_MACROS_HPP