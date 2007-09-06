/*!
    \file   SocketsManager.cpp
    \brief  SocketContainer�Q �Ǘ��N���X

    Copyright (c) 2005 Yamami
    All rights reserved.
    License=MIT/X License

    \author  Yamami
    \version $Revision$
    \date   create:2005/07/15 update:$Date$
*/

/*! \class SocketsManager
 *  \brief SocketContainer�Q �Ǘ��N���X
 */


#include <netprot/SocketsManager.h>

/*!
    \brief initialize
         SocketsManager �R���X�g���N�^
    \author Yamami
    \date   create:2004/10/11 update:$Date$
*/
SocketsManager::SocketsManager()
{
    //�R�l�N�V�����A����у��X���n�b�V�����C���X�^���X��
    conSocHash = new HashMap<SocketContainer*>(MAX_SOC_SIZE);
    lisSocHash = new HashMap<SocketContainer*>(MAX_SOC_SIZE);    
}


/*!
    \brief findSocket
         �\�P�b�g��������
    \param  uint32_t ip [in] ��IP�A�h���X
    \param  uint16_t  myport [in] ���ȃ|�[�g�ԍ�
    \param  uint16_t  youport[in] ����|�[�g�ԍ�
    \return SocketContainer*  ��������SocketContainer�ւ̃|�C���^�B������Ȃ��ꍇ��NULL�|�C���^
        
    \author Yamami
    \date   create:2005/07/20 update:
*/
SocketContainer* SocketsManager::findSocket(uint32_t ip ,uint16_t  myport ,uint16_t youport)
{
    char SocKey[30];    //SocketContainer�L�[
    SocketContainer *retSoc;     //SocketContainer���^�[��
    
    //�R�l�N�V�����\�P�b�g����T��
    //MAP�̃L�[��(String��)
    sprintf(SocKey , "%08x%08x%08x",myport,ip,youport);
    retSoc = conSocHash->get(SocKey);
    
    if(retSoc!=NULL){
        return retSoc;
    }
    
    //���X���\�P�b�g����T��
    //MAP�̃L�[��(String��)
    sprintf(SocKey , "%08x",myport);
    retSoc = lisSocHash->get(SocKey);
    
    if(retSoc!=NULL){
        return retSoc;
    }    

    return NULL;
}


/*!
    \brief registLisSocket
         ���X���\�P�b�g�o�^����
    \param  uint32_t tid [in] �X���b�hid
    \param  uint16_t  myport [in] ���ȃ|�[�g�ԍ�
    \return ����
        
    \author Yamami
    \date   create:2005/07/20 update:
*/
void SocketsManager::registLisSocket(uint32_t tid ,uint16_t  myport )
{
    SocketContainer *soc;
    
    soc = new SocketContainer();
    soc->tid = tid;
    soc->myport = myport;
    soc->status = LISTEN;

    char SocKey[30];    //SocketContainer�L�[
    
    sprintf(SocKey , "%08x",myport);
    lisSocHash->put(SocKey , soc);
}


/*!
    \brief registConSocket
         �R�l�N�V�����\�P�b�g�o�^����
    \param  uint32_t tid [in] �X���b�hid
    \param  uint16_t  myport [in] ���ȃ|�[�g�ԍ�
    \param  uint32_t ip [in] ��IP�A�h���X
    \param  uint16_t  youport[in] ����|�[�g�ԍ�    
    \return ����
        
    \author Yamami
    \date   create:2005/07/20 update:
*/
void SocketsManager::registConSocket(uint32_t tid ,uint16_t myport, uint32_t ip , uint16_t youport)
{
    SocketContainer *soc;
    
    soc = new SocketContainer();
    soc->tid = tid;
    soc->myport = myport;
    soc->ip = ip;
    soc->youport = youport; 
    soc->status = ESTABLISHED;

    char SocKey[30];    //SocketContainer�L�[
    
    sprintf(SocKey , "%08x%08x%08x",myport,ip,youport);
    conSocHash->put(SocKey , soc);
}