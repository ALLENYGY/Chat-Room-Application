

#include <bits/stdc++.h>

using namespace std;

string wechatIcon = R"(
                 ,EEEEEEi                         
               EEEEt..tEEEE                       
             LEE          GEE                     
            EE              EE                    
           EE                EE                   
          GE     G     tj     EE                  
          E     EEE    EEG     E                  
         iE     EE.    EE      EL                 
         EE                    LE                 
         Ei                  .LEEi                
         Et               iEEEEEEEEEG             
         EE             .EEj       .EEG           
         jE             EE           LEj          
          E            EG              E.         
          EE          EE   EEj    EE   tE         
           EE         E    EEi    EE    Et        
            EE       tE                 GE        
             EE      EE                 iE        
             EE EG   LE                 LE        
             EEELEEEEEE                 EL        
            EEE       EG                E         
            G          E               EG         
                       jEE            EE          
                        .EEj        iEE           
                          iEEEEEEEEE Ej           
                             .LEGi EEEE           
                                     tE           
)";

void red_print(string out)
{
    cout << "\033[31;1m" << out << "\033[0m" << endl;
}

void green_print(string out)
{
    cout << "\033[32;1m" << out << "\033[0m" << endl;
}

void blue_print(string out)
{
    cout << "\033[34;1m" << out << "\033[0m" << endl;
}

void printWechat()
{
    green_print(wechatIcon);
}

int main()
{
    printWechat();
    blue_print("Hint Message.");
    red_print("Hello, Wechat!");
    return 0;
}