#include "Stock_Exchange.h"
using namespace std;

int main()
{
    //since there is only one exchange, its a singleton class, and the objec is named appropriately
    Stock_Exchange singleton;
    singleton.Operate();

    return 0;
}