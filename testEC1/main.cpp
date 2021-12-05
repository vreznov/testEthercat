#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>

#include "ethercattype.h"
#include "nicdrv.h"
#include "ethercatbase.h"
#include "ethercatmain.h"
#include "ethercatdc.h"
#include "ethercatcoe.h"
#include "ethercatfoe.h"
#include "ethercatconfig.h"
#include "ethercatprint.h"

#include "kecm.h"

#define EC_TIMEOUTMON 500
using namespace std;

int main()
{
    cout << "start!" << endl;

    KECM kecm;
    kecm.run_test();

    return 0;
}
