/*!
    \file   Ne2000MoNic.cpp
    \brief  Ne2000�h���C�o�N���X

    Copyright (c) 2004 Yamami
    All rights reserved.
    License=MIT/X License

    \author  Yamami
    \version $Revision$
    \date   create:2004/07/28 update:$Date$
*/

/*! \class Ne2000MoNic
 *  \brief Ne2000�h���C�o�N���X
 */



#include "Ne2000MoNic.h"


/*!
    \brief initialize
         Ne2000MoNic �R���X�g���N�^
    \author Yamami
    \date   create:2004/08/08 update:
*/
Ne2000MoNic::Ne2000MoNic()
{

}


/*!
    \brief initialize
         Ne2000MoNic init
    \author Yamami
    \return int ���� 0:���� �A 0�ȊO:�ُ�
    \date   create:2004/08/12 update:
*/
int Ne2000MoNic::init() 
{

    int reti;
    

    //�v���C�x�[�g�����o������
    ne_ringbuf_status=0;
    ne_ringbuf_bound=0;
    ne_ringbuf_len=0;

    ne_rx_start=0;      /* ��M�p�P�b�g�{�̂̊J�n�A�h���X */
    frame_len=0;        /* ��M�p�P�b�g�{�̂̒��� */
    ne_rx_bound=0;      /* ��M��̋��E���W�X�^�l */
    ne_rx_write_p=0;    /* ��M�p�P�b�g�������݃A�h���X */
    ne_rx_sub_len=0;    /* �܂�Ԃ����̒��� */
    ne_rx_remain_len=0; /* �c��̒���(�܂�Ԃ����Ȃ��Ƃ��͖{�̂̒����Ɠ���) */
    
    ne_sizeof_test_pattern=20;

    //Ne2000 ���݊m�F
    reti = nic_probe();
    if(reti != 0 ){
        printf("Does Not Exist Ne2K!!!\n");
        return -1;
    }

    //������MAC�A�h���X��\�����Ă݂�
    //int i;
    //printf("MAC ADR:");
    //for(i=0 ;i < 6 ; i++){
    //    printf("%x ",ether_mac_addr[i]);
    //}
    //printf("\n");

    //Ne2000 ������
    nic_init();

    return 0;
}


/*!
    \brief frame_input
        NE2000 �f�[�^���̓��[�`��
        �{���́ANE2000����̊��荞�݂ɂăR�[�������B
        �e�X�g�v���O�����ł́A�����I��MonaMain �ŃR�[���B
    \param  void
    \return void

    \author Yamami
    \date   create:2004/08/03 update:$Date$
*/
void Ne2000MoNic::frame_input(void)
{

    uint8_t sts,*buf;
    //�o�E���_�����W�X�^ �ƁA�J�����g�y�[�W���W�X�^��8�r�b�g��
    //�f�[�^�ɃA�N�Z�X����ہA8�r�b�g�V�t�g����16�r�b�g���A�N�Z�X���s��
    uint16_t  bnd,cpg;

    buf=frame_buf;

    // Page 0
    outp8( NE_P0_COMMAND, NE_CR_STA );
    // sts <- ��M�X�e�[�^�X���W�X�^(Receive Status Reg)
    sts=inp8( NE_P0_RSR );

//Yamami �f�o�b�O
//printf("sts : %x\n",sts);

    if( ( sts & NE_RSTAT_OVER ) !=0 ){
        printf("FIFO OverFlow\n");
        return; // ��MFIFO�I�[�o�[�t���[����
    }

    if( ( inp8( NE_P0_ISR ) & NE_ISR_OVW ) !=0 ){
        printf("RING OverFlow\n");
        return; // ��M�����O�o�b�t�@�I�[�o�[�t���[
    }

    //  ��M����
    if( ( sts & NE_RSTAT_PRX ) ==0 ){
        printf("Not Exist Packet \n");
        return; //  ��M�p�P�b�g�Ȃ�
    }

    //�y�[�W�𖾎��I�ɐ؂�ւ��� bnd �� cpg ��ǂ�
    outp8(NE_P0_COMMAND, NE_CR_PS0 | NE_CR_STA); /* Page 0 */
    bnd=inp8( NE_P0_BNRY ) + 1;      // bnd <-bnd  
    //bnd=inp8( NE_P0_BNRY );      // bnd <-bnd ������+1���Ȃ�
    outp8(NE_P1_COMMAND, NE_CR_PS1 | NE_CR_STA); /* Page 1 */
    cpg=inp8( NE_P1_CURR );          // cpg <- Current Page
    
    //Page0�ɖ߂��Ă���
    outp8( NE_P0_COMMAND, NE_CR_PS0 );


//Yamami �f�o�b�O
//printf("bnd : %x\n",bnd);
//printf("cpg : %x\n",cpg);


    if( bnd == NE_RX_PAGE_STOP ){
        // if last page then set to start page
        bnd=NE_RX_PAGE_START;
    }
    if( cpg == NE_RX_PAGE_STOP ){
        /* if last page then set to start page */
        cpg=NE_RX_PAGE_START; 
    }
    if( cpg == bnd ){        // Current Page = bound ?
        //printf("Not Exist Packet buffer \n");
        return;         // = �Ȃ� �o�b�t�@��Ƀp�P�b�g�Ȃ�
    }

    
    // bound+1 �y�[�W�̐擪4�o�C�g��ǂݍ���
    // Yamami �����ƁA�A�������ϐ��錾���A�A������4�o�C�g�ł��鎖�ɗ����Ă���H�H
    // �o�b�t�@�Ɉ�U���[�h���āA������鏈���ɕύX
    //ne_pio_readmem( bnd << 8, &ne_ringbuf_status, 4 );

    uint8_t bndBuf[4];
    ne_pio_readmem( bnd << 8, bndBuf, 4 );

// Yamami�f�o�b�O ���[�h�A�h���X�̕\��
//printf("Read Src = bnd << 8 : %x\n",bnd << 8);

    ne_ringbuf_status = bndBuf[0]; /* Receive Status */
    ne_ringbuf_bound = bndBuf[1] & 0xFF; /* Next Packet Pointer */
    ne_ringbuf_len = bndBuf[3] * 256 + bndBuf[2];   /* Receive Byte Count */

    //�����ꂪ�A���ꂼ��t�̂悤�ȋC������B�G���f�B�A���̈Ⴂ�H�H
    //  ���̎��́ARead/Write����ύX
    //ne_ringbuf_status = bndBuf[1]; /* Receive Status */
    //ne_ringbuf_bound = bndBuf[0] & 0xFF; /* Next Packet Pointer */
    //ne_ringbuf_len = bndBuf[2] * 256 + bndBuf[3];   /* Receive Byte Count */

// Yamami�f�o�b�O
//printf("ne_ringbuf_status : %x\n",ne_ringbuf_status);
//printf("ne_ringbuf_bound  : %x\n",ne_ringbuf_bound);
//printf("ne_ringbuf_len : %x\n",ne_ringbuf_len);

    ne_rx_start=(bnd << 8) + 4; // �p�P�b�g�{�̂̊J�n�A�h���X

    // CRC�̕��̒���������
    // CRC�̕��̒��������� ? CRC����Ȃ���ne_ringbuf_*��4 uint8_t?
    frame_len=ne_ringbuf_len - 4; /* �p�P�b�g�{�̂̒��� */

    // ��M�I����̋��E���W�X�^�l
    ne_rx_bound=ne_ringbuf_bound;

    if( ( ne_ringbuf_status & NE_RSTAT_PRX ) !=0 ){
                    // ��M������I������
        if( frame_len >= ETHER_HEADER_SIZE ){
                    // �ŒZ�����Z���Ƃ��̓G���[
            if( frame_len < ETHER_MAX_PACKET ) {
                    // �ő咷��蒷���Ƃ��̓G���[

                ne_rx_remain_len=frame_len;

                // �p�P�b�g�̎�荞�ݏ���
                // �܂�Ԃ����̒���
                ne_rx_sub_len=NE_RX_PAGE_STOP * 256 - ne_rx_start;

// Yamami�f�o�b�O
//printf("frame_input 03 ne_rx_start=%x\n",ne_rx_start);
//printf("frame_input 04 ne_rx_sub_len=%x\n",ne_rx_sub_len);

                if( ne_rx_sub_len < frame_len ){
                    // ��M���ׂ��p�P�b�g�͐܂�Ԃ��Ă���
                    // �O�����̓ǂݍ���
                    ne_pio_readmem( ne_rx_start, buf, ne_rx_sub_len );
                    ne_rx_start=NE_RX_PAGE_START * 256;

                    // �������݃A�h���X�̍X�V
                    buf+=ne_rx_sub_len;
                    // �c��̓ǂݍ��ݒ��̍X�V
                    ne_rx_remain_len=frame_len - ne_rx_sub_len;
                }
                // �p�P�b�g�̓ǂݍ���
                ne_pio_readmem( ne_rx_start, buf, ne_rx_remain_len );
                
            }
            else{
                //printf("Error frame_len >= ETHER_MAX_PACKET \n");
            }
        }
        else{
            //printf("Error frame_len >= ETHER_HEADER_SIZE \n");
        }
    }
    else{
        //printf("Error ne_ringbuf_status & NE_RSTAT_PRX = 0 \n");
    }

    // Yamami �o�E���_�����W�X�^ �̍X�V
    bnd=ne_rx_bound; 
    if( bnd == NE_RX_PAGE_START ){
        bnd=NE_RX_PAGE_STOP;
    }
    bnd--;
    outp8( NE_P0_BNRY, bnd );    // ���E���W�X�^ = ���̃o�b�t�@ - 1

    //  ���荞�݃X�e�[�^�X���W�X�^�N���A
    outp8( NE_P0_ISR, 0xff );
    
    //H8 ���
    outp8(NE_P0_IMR, NE_IMR_PRXE); /* Packet Receive interrupt enable */

}



/*!
    \brief frame_output
        NE2000 �f�[�^�o�̓��[�`��
    \param  uint8_t *pkt [in] �f�[�^�p�P�b�g�ւ̃|�C���^
    \param  uint8_t *mac [in] �����MAC�A�h���X�ւ̃|�C���^
    \param  uint32_t size [in] �p�P�b�g�T�C�Y
    \param  uint16_t pid [in] �v���g�R��ID(ETHER_PROTO)
    \return void

    \author Yamami
    \date   create:2004/08/03 update:$Date$
*/
void Ne2000MoNic::frame_output( uint8_t *pkt, uint8_t *mac, uint32_t size, uint16_t pid )
{
    
    uint32_t        ptx_type=0;
    uint32_t        ptx_size=0;
    uint8_t       *ptx_packet=0;
    uint8_t       *ptx_dest=0;
    
    
    // ���M���������Ă��邩�ǂ����`�F�b�N����
    while( ( inp8( NE_P0_COMMAND ) & 0x04 ) !=0 );

//Yamami �f�o�b�O
//printf("frame_output 01\n");
    
    ptx_dest=mac;
    ptx_size=size;
    ptx_packet=pkt;
    // �l�b�g���[�N�o�C�g�I�[�_�[�ɕϊ�����
    // Yamami �ϊ��s�v�H
    ptx_type=(pid >> 8)+(pid << 8);

//Yamami �f�o�b�O
//int i;
//for(i=0 ; i<2 ; i++){
//    printf("ptx[1] = %x \n",(uint8_t *)(&ptx_type + 1));
//}


    // ���荞�݋֎~
    // ���M�������Ɏ�M����ƃ��W�X�^�������Ă��܂�
    //disableInterrupt();
    disableNetWork();

    // ����A�h���X�̏�������
    ne_pio_writemem( ptx_dest, NE_TX_PAGE_START << 8, 6 );
    // ���M���A�h���X�̏�������
    ne_pio_writemem( ether_mac_addr, ( NE_TX_PAGE_START << 8 ) + 6, 6 );
    // �v���g�R��ID�̏�������
    ne_pio_writemem( (uint8_t *)&ptx_type, ( NE_TX_PAGE_START << 8 ) + 12, 2 );
    // �f�[�^�����̏�������
    ne_pio_writemem( ptx_packet, ( NE_TX_PAGE_START << 8 ) + 14, ptx_size );

    ptx_size+=ETHER_HEADER_SIZE;

    // �ŏ��p�P�b�g�����Z�����ǂ������`�F�b�N����
    if( ptx_size < ETHER_MIN_PACKET ){
        ptx_size=ETHER_MIN_PACKET;
    }

    outp8( NE_P0_COMMAND, NE_CR_PS0 + NE_CR_RD2 + NE_CR_STA );
    
    // ���M�o�b�t�@�̈�̎w��
    outp8( NE_P0_TPSR, NE_TX_PAGE_START );
    
    // ���M���̎w��
    outp8( NE_P0_TBCR0, ptx_size & 0xff);
    outp8( NE_P0_TBCR1, ptx_size >> 8 );

    // ���M���߂𔭍s����
    outp8( NE_P0_COMMAND, NE_CR_PS0 + NE_CR_TXP + NE_CR_RD2 + NE_CR_STA );

    // ���荞�݋���
    //enableInterrupt();
    enableNetWork();

//Yamami �f�o�b�O
//printf("frame_output 02\n");

    // ���M���������Ă��邩�ǂ����`�F�b�N����
    // 2004/11/16 Yamami QEMU on Ne2000 ���Ƃ��̃`�F�b�N���i���ɒʂ�Ȃ��悤�Ȃ̂Ń`�F�b�N���Ȃ�
    //while( ( inp8( NE_P0_COMMAND ) & 0x04 ) !=0 );

//Yamami �f�o�b�O
//printf("frame_output 03\n");

}


/*!
    \brief nic_probe
        NE2000�������[�`��
        �o�b�t�@�������ɏ������݂Ɠǂݍ��݂��s���ANE2000�����݂��邱�Ƃ��m�F����
    \param  void
    \return void

    \author Yamami
    \date   create:2004/08/01 update:$Date$
*/
int Ne2000MoNic::nic_probe(void)
{
    int i;

    /* �\�t�g�E�F�A���Z�b�g */
    //outp8( NE_ASIC_RESET, inp8( NE_ASIC_RESET ) ); 
    /* ���Z�b�g�����܂ő҂� */
    //ne_wait_func(0);

    /* DMA ��������~����B */
    outp8( NE_P0_COMMAND, NE_CR_RD2 + NE_CR_STP );

    /* ��~���鍠�܂ő҂� */
    //ne_wait_func(0);

    //  �p�P�b�g���������ɏ�����Ȃ��悤�ɂ���
    outp8( NE_P0_RCR, NE_RCR_MON );

    // ���[�h���]���A���[�v�o�b�N���[�h
    outp8( NE_P0_DCR, NE_DCR_WTS + NE_DCR_FT1 + NE_DCR_LS );

    // ��M�o�b�t�@�J�n�A�h���X
    // 64
    outp8( NE_P0_PSTART, NE_MEM_START );

    // ��M�o�b�t�@�I���A�h���X
    // 128
    outp8( NE_P0_PSTOP, NE_MEM_END );

    // �������e�X�g�p�^�[����������
    ne_pio_writemem( (uint8_t *)ne_test_pattern, NE_MEM_START * NE_PAGE_SIZE, ne_sizeof_test_pattern );
    // �������e�X�g�p�^�[���ǂݍ���
    ne_pio_readmem( NE_MEM_START * NE_PAGE_SIZE, ne_test_buffer, ne_sizeof_test_pattern );
    // �e�X�g�p�^�[���̔�r
    if( ne_bcompare( (uint8_t *)ne_test_pattern, ne_test_buffer, ne_sizeof_test_pattern )!=0 )
        return(1);  // �s��v�Ȃ�I��

    // EEPROM �f�[�^�ǂ݂���
    ne_pio_readmem( 0, ne_test_buffer, 16 );

    // �C�[�T�l�b�g�A�h���X�擾
    for(i=0;i<11;i+=2)
        ether_mac_addr[i/2]=ne_test_buffer[i];

    // ���荞�݃X�e�[�^�X���W�X�^�N���A
    outp8( NE_P0_ISR, 0xff );

    return(0);
}


/*!
    \brief nic_init
        NE2000���������[�`��
    \param  void
    \return void

    \author Yamami
    \date   create:2004/07/28 update:$Date$
*/
void Ne2000MoNic::nic_init(void)
{
    // �e�ϐ��̏�����
    int i;
    uint8_t c;

    //NIC���Z�b�g
    c = inp8(NE_ASIC_RESET);
    outp8(NE_ASIC_RESET, c);
    
    //���Z�b�g�����܂ő҂�
    sleep(300);
    
    // �����[�gDMA ��~
    outp8( NE_P0_COMMAND, ne_cr_proto | NE_CR_STP );

    // FIFO �X���b�V�����h 8Byte,�����[�gDMA �����������֎~
    // 8086 �o�C�g�I�[�_,16bit�� DMA �]�� 
    // Page0_0Eh DATA CONFIGURATION REGISTER (DCR) 0EH (WRITE)
    //   7 6   5   4   3  2   1    0
    //   - FT1 FT0 ARM LS LAS BOS WTS
    //   0 1   0   0   1   0   0    1
    outp8( NE_P0_DCR, NE_DCR_FT1 + NE_DCR_WTS + NE_DCR_LS );

    // �����[�gDMA �o�C�g�J�E���^�N���A
    outp8( NE_P0_RBCR0, 0 );
    outp8( NE_P0_RBCR1, 0 );

    // ���j�^���[�h
    // (�p�P�b�g��M�����������Ȃ����A�o�b�t�@�����O�͂��Ȃ�)
    // RECEIVE CONFIGURATION REGISTER (RCR) 0CH (WRITE)
    outp8( NE_P0_RCR, NE_RCR_MON );

    // �������[�v�o�b�N���[�h
    // TRANSMIT CONFIGURATION REGISTER (TCR) 0DH (WRITE)
    outp8( NE_P0_TCR, NE_TCR_LB0 );

    // ���M�����O�o�b�t�@�J�n�A�h���X�̐ݒ�
    // 64
    outp8( NE_P0_TPSR, NE_TX_PAGE_START );
    // ��M�����O�o�b�t�@�J�n�A�h���X�̐ݒ� 
    // 70
    outp8( NE_P0_PSTART, NE_RX_PAGE_START );
    
    // ��M�����O�o�b�t�@���E�A�h���X�̐ݒ�
    // 70
    outp8( NE_P0_BNRY, NE_RX_PAGE_START );
    // ��M�����O�o�b�t�@�I���A�h���X�̐ݒ�
    // 128
    outp8( NE_P0_PSTOP, NE_RX_PAGE_STOP );

    // ���荞�݃X�e�[�^�X���W�X�^�̃N���A
    outp8( NE_P0_ISR, 0xff );
    //  ���荞�݋������̐ݒ� 
    // Packet recieve successful
    outp8( NE_P0_IMR, NE_IMR_PRXE );
    
    //Yamami �S���荞�݂������Ă݂�
    //outp8( NE_P0_IMR, 0x7F );

    // Page 1 �̐ݒ�
    outp8( NE_P0_COMMAND, ne_cr_proto | ( NE_CR_PS1 + NE_CR_STP ) );

    // Ethernet �A�h���X�̐ݒ�
    // �����Ŏw�肵���A�h���X�̃p�P�b�g���󂯎��
    for(i=0;i<6;i++){
        outp8( NE_P1_PAR0 + i, ether_mac_addr[i] );
    }

    // �ŏ��Ɏ�M�����p�P�b�g���i�[����A�h���X�̐ݒ�
    outp8( NE_P1_CURR, NE_RX_PAGE_START + 1 );

    /* �}���`�L���X�g���W�X�^�̐ݒ� */
    outp8( NE_P1_MAR0, 0 );
    outp8( NE_P1_MAR0+1, 0 );
    outp8( NE_P1_MAR0+2, 0 );
    outp8( NE_P1_MAR0+3, 0 );
    outp8( NE_P1_MAR0+4, 0 );
    outp8( NE_P1_MAR0+5, 0 );
    outp8( NE_P1_MAR0+6, 0 );
    outp8( NE_P1_MAR0+7, 0 );

    // Page 0 �ɂ��ǂ� 
    outp8( NE_P0_COMMAND, ne_cr_proto | NE_CR_STP );

    // ��M�p�P�b�g�t�B���^�̐ݒ�
    // �u���[�h�L���X�g�Ǝ������݂̂��������Ɋi�[ 
    // accept broadcast
    outp8( NE_P0_RCR, NE_RCR_AB );
// �� ���R�����g�A�E�g�����
// Yamami���j�^���[�h�̂܂܂Ƃ��Ă݂�!!!!! ���ɂȂ�

    //Yamami �v���~�X�L���X�g�̂ݎ󂯎��悤�ɂ��Ă݂�???? �Ⴄ�悤��
    //outp8( NE_P0_RCR, NE_RCR_APROMIS );

    // NIC ���A�N�e�B�u�ɂ���
    outp8( NE_P0_COMMAND, ne_cr_proto | NE_CR_STA );

    // ���[�v�o�b�N���[�h�𔲂��Ēʏ퓮�샂�[�h�ɓ��� 
    outp8( NE_P0_TCR, 0 );
}





/*!
    \brief ne_pio_writemem
        ����J���[�`��
        Ne2000 �o�b�t�@��������������
    \param  uint8_t *src [in] �]�����A�h���X
    \param  uint32_t dest [in] �]����A�h���X
    \param  uint32_t size [in] ���� 
    \return void

    \author Yamami
    \date   create:2004/08/02 update:$Date$
*/
void Ne2000MoNic::ne_pio_writemem( uint8_t *src, uint32_t dest, uint32_t size )
{
    uint32_t i;

    uint16_t writetmp;

    /* �X�e�[�^�X���W�X�^�N���A */
    outp8( NE_P0_COMMAND, NE_CR_RD2 + NE_CR_STA );
    outp8( NE_P0_ISR, NE_ISR_RDC);

    /* ���� */
    outp8( NE_P0_RBCR0, size & 0xff );
    outp8( NE_P0_RBCR1, size >> 8 );

    /* �]����A�h���X */
    outp8( NE_P0_RSAR0, dest & 0xff );
    outp8( NE_P0_RSAR1, dest >> 8 );
    outp8( NE_P0_COMMAND, NE_CR_RD1 + NE_CR_STA );

//    for(i=0;i<size;i+=2){
//        outp8( PIO_ADATA, *(src+1) );
//        outp8( NE_ASIC_DATA, *src );
//        src+=2;
//    }


    // 2004/08/02 DATA��16�r�b�g���ł��Ƃ肷��̂ŁAWord�ϊ�����I/O
    for(i = 0 ; i < size ; i+=2 , src+=2){
        //writetmp = (uint16_t)(*(src) << 8) + (uint16_t)*(src+1);
        //���g���G���f�B�A���Ȃ炱���H�H
        writetmp = (uint16_t)(*(src + 1) << 8) + (uint16_t)*(src);
        outp16( NE_ASIC_DATA, writetmp );
    }
    
    /* wait */
    for(i=0;i<0xff;i++){
        if( ( inp8(NE_P0_ISR) & NE_ISR_RDC ) == 0 )
            break;
    }

}


/*!
    \brief ne_pio_readmem
        ����J���[�`��
         NE2000 �̃���������ǂ݂���
    \param  uint32_t src [in] �]�����A�h���X
    \param  uint8_t *dest [in] �]����A�h���X
    \param  uint32_t size [in] ���� 
    \return void

    \author Yamami
    \date   create:2004/08/02 update:$Date$
*/
void Ne2000MoNic::ne_pio_readmem( uint32_t src, uint8_t *dest, uint32_t size )
{
    uint32_t i;

    uint16_t readtmp;

//Yamami �f�o�b�O
//printf("ne_pio_readmem src=%x \n",src);

    // abort DMA, start NIC
    outp8( NE_P0_COMMAND, NE_CR_RD2 + NE_CR_STA );
    // length
    outp8( NE_P0_RBCR0, size & 0xff );
    outp8( NE_P0_RBCR1, size >> 8 );
    // source address
    outp8( NE_P0_RSAR0, src & 0xff );
    outp8( NE_P0_RSAR1, src >> 8 );
    outp8( NE_P0_COMMAND, NE_CR_RD0 + NE_CR_STA );

//    for(i=0;i<size;i+=2){
//        *dest=inp8( NE_ASIC_DATA );
//        *(dest+1)=inp8( PIO_ADATA );
//        dest+=2;
//    }
    
    // 2004/08/02 DATA��16�r�b�g���ł��Ƃ肷��̂ŁAWord�ϊ�����I/O
    for(i = 0 ; i < size ; i+=2 , dest+=2){
        readtmp=inp16( NE_ASIC_DATA );
        //*dest=(uint8_t)(readtmp >> 8);
        //*(dest+1)=(uint8_t)(readtmp & 0xff);
        //���g���G���f�B�A���Ȃ炱���H�H
        *(dest+1)=(uint8_t)(readtmp >> 8);
        *(dest)=(uint8_t)(readtmp & 0xff);
    }
    
}



/*!
    \brief ne_bcompare
        ����J���[�`��
         �o�C�i����r���[�`��
    \param  uint8_t *src [in] ��r���A�h���X
    \param  uint8_t *dest [in] ��r��A�h���X
    \param  uint32_t size [in] ����
    \return int ����:��v==0,�s��v==0�ȊO

    \author Yamami
    \date   create:2004/08/02 update:$Date$
*/
int Ne2000MoNic::ne_bcompare( uint8_t *src, uint8_t *dest, uint32_t size )
{
    uint32_t i;

    for(i=0;i<size;i++){
        if( src[i]!=dest[i] )
            return(1);
    }

    return(0);
}



/*!
    \brief getNicIRQ
       NIC IRQ �Q�b�^�[
    \author Yamami
    \date   create:2004/08/30 update:
*/
int Ne2000MoNic::getNicIRQ() 
{

    //TODO Yamami!! �{���́A�ݒ�t�@�C��������IRQ���𓾂�
    return nicIRQ;
}

/*!
    \brief setNicIRQ
       NIC IRQ �Z�b�^�[
    \author Yamami
    \date   create:2004/10/31 update:2004/10/31
*/
void Ne2000MoNic::setNicIRQ(int value) 
{
    nicIRQ = value;

}


/*!
    \brief getNicIOBASE
       NIC IO_BASE �Q�b�^�[
    \author Yamami
    \date   create:2004/10/31 update:2004/10/31
*/
int Ne2000MoNic::getNicIOBASE() 
{

    //TODO Yamami!! �{���́A�ݒ�t�@�C��������IRQ���𓾂�
    return nicIo_Base;
}

/*!
    \brief setNicIOBASE
       NIC IO_BASE  �Z�b�^�[
    \author Yamami
    \date   create:2004/10/31 update:2004/10/31
*/
void Ne2000MoNic::setNicIOBASE(int value) 
{
    nicIo_Base = value;

}