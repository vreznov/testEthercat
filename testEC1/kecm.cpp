#include "kecm.h"

KECM::KECM()
{
    _IOMap.resize(4196);

    _usdo.resize(128);
}

KECM::~KECM()
{

}

void KECM::testInit()
{
    char fname[256] = {};
    strcpy(fname, m_ethName.c_str());
    if(!ec_init(fname)) {
        cout << "ec init failed" << endl;
        return;
    }

    cout << "ec init sucess" << endl;
    if(ec_config_init(false) <= 0) {
        cout << "ec_config_init failed" << endl;
    }

    cout << ec_slavecount << "slaves found" << endl;
    ec_config_map(_IOMap.data());
    ec_configdc();

    cout << "Slaves mapped, state to SAFE_OP." << endl;
    //等待所有从站切换为SAFE OP状态
    ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 4);

    //获取IO字节数
    int oloop, iloop;
    oloop = ec_slave[0].Obytes;
    iloop = ec_slave[0].Ibytes;
    if(oloop == 0 and ec_slave[0].Obits > 0) oloop = 1;
    if(iloop == 0 and ec_slave[0].Ibits > 0) iloop = 1;
    cout << "oloop: " << oloop << "\tiloop: " << iloop << endl;

    cout << "segments: " << ec_group[0].nsegments\
         << ec_group[0].IOsegment[0]\
         << ec_group[0].IOsegment[1] << endl;

    cout << "request operational state for all slaves" << endl;
    _expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
    cout << "expectedWKS is: " << _expectedWKC << endl;

    ec_slave[0].state = EC_STATE_OPERATIONAL;

    //?send valid process data to make outputs in slaves happy
    ec_send_processdata();
    ec_receive_processdata(EC_TIMEOUTRET);

    //request OP state for all slaves
    ec_writestate(0);

    int chk = 40;  //check次数
    //等待所有从站进入OP状态
    do{
        ec_send_processdata();
        ec_receive_processdata(EC_TIMEOUTRET);
        ec_statecheck(0, EC_STATE_OPERATIONAL, 50000);
    }
    while (chk-- and (ec_slave[0].state != EC_STATE_OPERATIONAL));

    //所有从站进入OP状态
    if(ec_slave[0].state == EC_STATE_OPERATIONAL) {
        cout << "all slaves reached OP state" << endl;
        _state = 1;

        //循环收发
        for(int i=0; i<1000; i++) {
            ec_send_processdata();
            _wkc = ec_receive_processdata(EC_TIMEOUTRET);

            //所有数据读写完毕
            if(_wkc >= _expectedWKC) {
//                cout << "processdata cycle " << i << ", WKC " << _wkc;

                printf("processdata cycle %d, WKC %d", i, _wkc);
                //显示所有O数据
                for(int j=0; j<oloop; j++) {
                    printf(" %2.2x", ec_slave[0].outputs[j]);
                }
                //显示所有I数据
                for(int j=0; j<iloop; j++) {
                    printf(" %2.2x", ec_slave[0].inputs[j]);
                }

//                cout << "T:" << ec_DCtime << "\r";

                printf("T: %lld\r", ec_DCtime);
                _needlf = true;
            }
            usleep(5000);
        }

        _state = 0;
    }
    else {
        //暂时未处理，需要针对个别从站未能进入OP状态进行显示
        ec_readstate();
    }

    cout << "end simple test, close socket" << endl;
    ec_close();
}

void KECM::run_test()
{
    testInit();
}

string KECM::sdo2string(uint16 f_slave, uint16 f_index, uint8 f_subIndex, uint16 dtype)
{
    int l = _usdo.size() - 1;
    int i;

    uint8 *u8;
    int8 *i8;
    uint16 *u16;
    int16 *i16;
    uint32 *u32;
    int32 *i32;
    uint64 *u64;
    int64 *i64;
    float *sr;
    double *dr;
    char es[32];

    fill(_usdo.begin(), _usdo.end(), 0);

    ec_SDOread(f_slave, f_index, f_subIndex, false, &l, _usdo.data(), EC_TIMEOUTRXM);
    if(EcatError) {
        return ec_elist2string();
    }
    else {
        stringstream ss;
        switch (dtype) {
        case ECT_BOOLEAN:
            u8 = (uint8*)_usdo.data();
            if(_usdo[0]) ss << "TRUE";
            else ss << "FALSE";
            break;

        case ECT_INTEGER8:
            i8 = (int8*)_usdo.data();
            ss << hex << _usdo[0] << dec << _usdo[0];
            break;
        case ECT_INTEGER16:
            i16 = (int16*)_usdo.data();
            ss << hex << *i16 << dec << *i16;
            break;
        case ECT_INTEGER32:
        case ECT_INTEGER24:
            i32 = (int32*)_usdo.data();
            ss << hex << *i32 << dec << *i32;
            break;
        case ECT_INTEGER64:
            i64 = (int64*)_usdo.data();
            ss << hex << *i64 << dec << *i64;
            break;
        case ECT_UNSIGNED8:
            u8 = (uint8*)_usdo.data();
            ss << hex << *u8 << dec << *u8;
            break;
        case ECT_UNSIGNED16:
            u16 = (uint16*)_usdo.data();
            ss << hex << *u16 << dec << *u16;
            break;
        case ECT_UNSIGNED24:
        case ECT_UNSIGNED32:
            u32 = (uint32*)_usdo.data();
            ss << hex << *u32 << dec << *u32;
            break;
        case ECT_UNSIGNED64:
            u64 = (uint64*)_usdo.data();
            ss << hex << *u64 << dec << *u64;
            break;
        case ECT_REAL32:
            sr = (float*)_usdo.data();
            ss.precision(2);
            ss << fixed << *sr;
            break;
        case ECT_REAL64:
            dr = (double*)_usdo.data();
            ss.precision(4);
            ss << fixed << *dr;
            break;
        case ECT_BIT1:
        case ECT_BIT2:
        case ECT_BIT3:
        case ECT_BIT4:
        case ECT_BIT5:
        case ECT_BIT6:
        case ECT_BIT7:
        case ECT_BIT8:
           u8 = (uint8*)_usdo.data();
           ss << hex << *u8;
           break;
        case ECT_VISIBLE_STRING:
            ss << _usdo.data();
            break;
        case ECT_OCTET_STRING:
            ss << "ECT_OCTET_STRING";
            break;
        default:
            ss << "unknow type";
        }
        ss >> _hstr;
    }

    return _hstr;
}

int KECM::si_PDOAssign(uint16 f_slave, uint16 f_PDOAssign, int f_mapOffset, int f_bitOffset)
{
etohs
}

void KECM::ecatCheck(void *ptr)
{
    int slave;

    if((_state == 1) and (_wkc < _expectedWKC \
                         or ec_group[_currentGrooup].docheckstate)) {
        if(_needlf) {
            _needlf = false;
        }

        //一个或多个从站未响应
        ec_group[_currentGrooup].docheckstate = false;
        ec_readstate();

        for(slave = 1; slave < ec_slavecount; slave++) {
            if((ec_slave[slave].group == _currentGrooup) \
                    and (ec_slave[slave].state != EC_STATE_OPERATIONAL)) {
                ec_group[_currentGrooup].docheckstate = true;

                //根据不同的状态处理
                if(ec_slave[slave].state == \
                        (EC_STATE_SAFE_OP + EC_STATE_ERROR)) {
                    cout << "slave " << slave << "is in SAFE OP, attempting ack"\
                         << endl;
                    ec_slave[slave].state = (EC_STATE_SAFE_OP + EC_STATE_ACK);
                    ec_writestate(slave);
                }
                else if(ec_slave[slave].state == EC_STATE_SAFE_OP) {
                    cout << "slave " << slave << "is in SAFE OP, chenge to OP"\
                         << endl;
                    ec_slave[slave].state = EC_STATE_OPERATIONAL;
                    ec_writestate(slave);
                }
                else if(ec_slave[slave].state > 0){  //?
                    if(ec_reconfig_slave(slave, EC_TIMEOUTMON)) {
                        ec_slave[slave].islost = false;
                        cout << "slave " << slave << " reconfigured" << endl;
                    }
                }
                else if (!ec_slave[slave].islost) {
                    //recheck state
                    ec_statecheck(slave, EC_STATE_OPERATIONAL, EC_TIMEOUTRET);
                    if(!ec_slave[slave].state) {
                        ec_slave[slave].islost = true;
                        cerr << "slave " << slave << " is lost" << endl;
                    }
                }
            }

            if(ec_slave[slave].islost) {
                if(!ec_slave[slave].state) {
                    if(ec_recover_slave(slave, EC_TIMEOUTMON)) {
                        ec_slave[slave].islost = false;
                        cout << "MESSAGE: slave " << slave << "recovered" << endl;
                    }
                }
                else {
                    ec_slave[slave].islost = false;
                    cout << "slave " << slave << " found" << endl;
                }
            }
        }

        if(!ec_group[_currentGrooup].docheckstate) {
            cout << "all slaves resumed OP" << endl;
        }
    }

    usleep(10000);
}
