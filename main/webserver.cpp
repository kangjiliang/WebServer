#include "webserver.h"

int main()
{
    CWebServerRequst* webreq = new CWebServerRequst("./");
    CWebServerThreadPool webserver("0.0.0.0", 8888, webreq);
    webserver.startup();
}
