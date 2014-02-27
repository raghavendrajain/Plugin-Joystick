#include "SIGService.h"
#include <boost/python.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <iostream>
#include <string>


namespace py = boost::python;  
using namespace std; 




std::string parse_python_exception(); // functional declaration for exception handling

template <typename T> string tostr(const T& t) { ostringstream os; os<<t; return os.str(); } // template to convert double variables to string



class JoyStick_Service : public sigverse::SIGService  
  {  
public:  
    JoyStick_Service(std::string name);  
    ~JoyStick_Service();  
    double onAction();  
    void onRecvMsg(sigverse::RecvMsgEvent &evt);  
//private: Joystick*  JS; //pointer to joystick object  
//private: HWND hWnd;   
 };  
  JoyStick_Service::JoyStick_Service(std::string name) : SIGService(name){  
            //JS = new Joystick();  
            //hWnd = NULL;  
    };  
JoyStick_Service::~JoyStick_Service()  
 {  
    this->disconnect();  
 } 
void JoyStick_Service::onRecvMsg(sigverse::RecvMsgEvent &evt)  
{  
}

double JoyStick_Service::onAction()  
{  

	Py_Initialize();
     
	static int count=1;
	cout << count << endl; 

	try{


		py::object main_module = py::import("__main__");  
		py::object main_namespace = main_module.attr("__dict__");
		
		if (count == 1){
		py::exec("import Joystick as joy", main_namespace);
		py::exec("textPrint=joy.TextPrint()", main_namespace);
		py::exec("a=textPrint.joystick()", main_namespace);

		}

	
		float dataArray[3]={0,0,0};
		float buttonArray[3]={0,0};
		std::string msgFromWii("");
		char tmp[128]={0};

		py::object listOfPos = py::eval("next(a)", main_namespace);


	    int lenList=len((listOfPos));
		//std::cout << " length is " << lenList <<std::endl;
		if (lenList==3){
			for(unsigned int i=0; i<lenList; i++){
				//std::cout << py::extract<double>((listOfPos)[i])<< std::endl;
				dataArray[i] = py::extract<double>((listOfPos)[i]);	
					}
		sprintf(tmp,  "%f,%f,%f", dataArray[0], dataArray[1], dataArray[2]);
		//std::cout<<"At i: " << i <<std::endl; 
		std::cout<<"The data is" << dataArray[0] << " , " << dataArray[1] << " , " << dataArray[2]<<std::endl;
		msgFromWii = std::string(tmp);
				}
		else  {
			buttonArray[0]= py::extract<double>((listOfPos)[0]);
			buttonArray[1]=py::extract<double>((listOfPos)[1]);
			sprintf(tmp,  "%f,%f",  buttonArray[0],  buttonArray[1]);
			//std::cout<<"At i: " << i <<std::endl; 
			std::cout<<"The data is" <<  buttonArray[0] << " , " <<  buttonArray[1] <<std::endl;
			msgFromWii = std::string(tmp);
			std::cout << "The sent message from Wii is: " << msgFromWii << std::endl;
			
			}


		//sprintf(tmp,  "%f,%f,%f", dataArray[0], dataArray[1], dataArray[2]);
		//std::cout<<"At i: " << i <<std::endl; 
		//std::cout<<"The data is" << dataArray[0] << " , " << dataArray[1] << " , " << dataArray[2]<<std::endl;
		//msgFromWii = std::string(tmp);
		//std::cout << "The sent message from Wii is: " << msgFromWii << std::endl; 
		//srv.sendMsg("robot_test", msgFromWii);
		this->sendMsg("robot_000", msgFromWii);
		
	}

    catch(boost::python::error_already_set const &){
        // Parse and output the exception
        std::string perror_str = parse_python_exception();
        std::cout << "Error in Python: " << perror_str << std::endl;
    }

  count ++ ; 
  return 0.02;

}

int main(int argc, char** argv)
{
	// Create an instance of the service class with the specified service name
	JoyStick_Service srv("MouseService");
	std::string host = argv[1];
	unsigned short port = (unsigned short)(atoi(argv[2]));  
    srv.connect(host, port); 
	srv.startLoop();
	// disconnect from the server
	//srv.disconnect();

	return 0;
}




std::string parse_python_exception(){
    PyObject *type_ptr = NULL, *value_ptr = NULL, *traceback_ptr = NULL;
    // Fetch the exception info from the Python C API
    PyErr_Fetch(&type_ptr, &value_ptr, &traceback_ptr);

    // Fallback error
    std::string ret("Unfetchable Python error");
    // If the fetch got a type pointer, parse the type into the exception string
    if(type_ptr != NULL){
        py::handle<> h_type(type_ptr);
        py::str type_pstr(h_type);
        // Extract the string from the boost::python object
        py::extract<std::string> e_type_pstr(type_pstr);
        // If a valid string extraction is available, use it 
        //  otherwise use fallback
        if(e_type_pstr.check())
            ret = e_type_pstr();
        else
            ret = "Unknown exception type";
    }
    // Do the same for the exception value (the stringification of the exception)
    if(value_ptr != NULL){
        py::handle<> h_val(value_ptr);
        py::str a(h_val);
        py::extract<std::string> returned(a);
        if(returned.check())
            ret +=  ": " + returned();
        else
            ret += std::string(": Unparseable Python error: ");
    }
    // Parse lines from the traceback using the Python traceback module
    if(traceback_ptr != NULL){
        py::handle<> h_tb(traceback_ptr);
        // Load the traceback module and the format_tb function
        py::object tb(py::import("traceback"));
        py::object fmt_tb(tb.attr("format_tb"));
        // Call format_tb to get a list of traceback strings
        py::object tb_list(fmt_tb(h_tb));
        // Join the traceback strings into a single string
        py::object tb_str(py::str("\n").join(tb_list));
        // Extract the string, check the extraction, and fallback in necessary
        py::extract<std::string> returned(tb_str);
        if(returned.check())
            ret += ": " + returned();
        else
            ret += std::string(": Unparseable Python traceback");
    }
    return ret;
}
