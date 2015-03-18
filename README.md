# Some Small Library 

This small library designed for simplify coding in C++ under unix like or windows os.

Lib containing code page converters:
  ConvertCodePageString - function for convert from any code page string to other code page string, 
    includings unicode or multibyte conversion.

and adding stream like operations:
 different  << operators for adding various data.

class EX_WND for simplification work with windows descriptors. 

Example using:
  EX_WND MainWindow = hWnd;                   //takes from standart HWND
  
  MainWindow[IDC_EDIT1].Text = 23;  
  
  MainWindow[IDC_EDIT2].Text = "Hello world"; //Stand. analog:
                                              //SetWindowTextA(GetDlgItem(hWnd,IDC_EDIT2),"Hello world");

  int g = MainWindow[IDC_EDIT2].Text.length;  //return: 11

  HWND hParent = MainWindow.Parent;           //Analog: HWND hParent = GetParent(hWnd);

  int i = 1, * MyData = &i, *GettedData;
  
  MainWindow.UserData = MyData;               //Analog: SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)MyData);
  GettedData = MainWindow.UserData;           //instead of: GettedData =(int*)GetWindowLongPtr(hWnd, GWL_USERDATA);
  
