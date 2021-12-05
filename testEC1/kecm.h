#ifndef KECM_H
#define KECM_H

/**********************************************************
 * 测试SOEM
 *
 *
 * ********************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>

#include "ethercattype.h"
#include "nicdrv.h"
#include "ethercatbase.h"
#include "ethercatmain.h"
#include "ethercatdc.h"
#include "ethercatcoe.h"
#include "ethercatfoe.h"
#include "ethercatconfig.h"
#include "ethercatprint.h"

#define EC_TIMEOUTMON 500

using namespace std;

class KECM
{
public:
    KECM();
    ~KECM();

    void testInit();

    void run_test();

    //! 读取SDO信息
    string sdo2string(uint16 f_slave, uint16 f_index, uint8 f_subIndex, uint16 dtype);

    //! 读取PDO结构
    int si_PDOAssign(uint16 f_slave, uint16 f_PDOAssign, int f_mapOffset,\
                     int f_bitOffset);

private:
    string m_ethName = "enp4s0";
    vector<char> _IOMap;
    int _state = 0;  //主站状态 1-OP
    int _wkc;
    int _expectedWKC;
    int _currentGrooup = 0;
    bool _needlf = false;
    ec_ODlistt _ODlist;
    ec_OElistt _OElist;

    vector<char> _usdo;
    string _hstr;

    OSAL_THREAD_FUNC ecatCheck(void *ptr);
};

#endif // KECM_H
