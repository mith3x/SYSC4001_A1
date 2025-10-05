/**
 *
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 *
 */

#include<interrupts.hpp>

int main(int argc, char** argv) {

    #include <interrupts.hpp>
    
    //vectors is a C++ std::vector of strings that contain the address of the ISR
    //delays  is a C++ std::vector of ints that contain the delays of each device
    //the index of these elemens is the device number, starting from 0
    auto [vectors, delays] = parse_args(argc, argv);
    std::ifstream input_file(argv[1]);

    std::string trace;      //!< string to store single line of trace file
    std::string execution;  //!< string to accumulate the execution output

    /******************ADD YOUR VARIABLES HERE*************************/

    int current_time = 0; //this will keep track of the clock
    std::vector<int> device_finish_time(delays.size(), -1); 
    int CSR_var = 10;     // context save/restore time (change to 20, 30 later)
    int ISR_var = 40;     // ISR body time (change to 80, 120, 160, 200 later)
    /******************************************************************/

    //parse each line of the input trace file
    while(std::getline(input_file, trace)) {
        auto [activity, duration_intr] = parse_trace(trace);

        /******************ADD YOUR SIMULATION CODE HERE*************************/
        if (activity == "CPU"){ // then we log CPU Burst
            execution += std:: to_string(current_time) + ", " + std:: to_string(duration_intr) + " , CPU burst\n";
            current_time += duration_intr;

        }else if (activity == "SYSCALL"){ // we log a system call
            int d = duration_intr; 

            //here we are switching to kernel
            execution += std:: to_string (current_time) + ", 1, switch to kernel mode\n";
            current_time += 1;
            
            //saving the context
            execution += std:: to_string(current_time) + ", " + std:: to_string(CSR_var) + ", context saved\n";
            current_time += CSR_var;

            //finding the vector position
            int pos = d * 4; //since its 2 bytes each
            execution += std:: to_string(current_time) + ", 1, find vector " + std:: to_string(d) + " in memory position " + std:: to_string(pos) + "\n";
            current_time+= 1;

            //fetching ISR addr
            execution += std:: to_string(current_time) + ", 1, obtain ISR address " + vectors[d] + "\n";
            current_time += 1;

            //ISR execute
            execution += std:: to_string(current_time) + ", " + std:: to_string(ISR_var) + " call device driver " + std:: to_string(d) + "\n";
            current_time +=ISR_var;

            //IRET
            execution += std:: to_string(current_time) + ", 1 IRET\n";
            current_time +=1;

            //device finishing 
            int finish_time = current_time + delays[d];
            execution += std:: to_string(current_time) + ", 0, scheduled END I/O " + std:: to_string(d) + " at t= " + std::to_string(finish_time) + "\n";
            device_finish_time[d] = finish_time;

        }else if (activity == "END_IO"){
            int d = duration_intr;
            if (current_time < device_finish_time[d]){
                int wait_time = device_finish_time[d] - current_time;
                execution += std:: to_string(current_time) + ", " + std:: to_string(wait_time) + ", wait for device " + std:: to_string(d) + " completion\n";
                current_time = device_finish_time[d];
            }

            //Interrupt occurance
            execution += std:: to_string(current_time) + ", 1, end of I/O " + std:: to_string(d) + ": interrupt\n";
            current_time +=1;

            //here we are switching to kernel
            execution += std:: to_string (current_time) + ", 1, switch to kernel mode\n";
            current_time +=1;
            
            //saving the context
            execution += std:: to_string(current_time) + ", " + std:: to_string(CSR_var) + ", context saved\n";
            current_time += CSR_var;

            //finding the vector position
            int pos = d * 4; //since its 2 bytes each
            execution += std:: to_string(current_time) + ", 1, find vector " + std:: to_string(d) + " in memory position " + std:: to_string(pos) + "\n";
            current_time+=1;

            //fetching ISR addr
            execution += std:: to_string(current_time) + ", 1, obtain ISR address " + vectors[d] + "\n";
            current_time +=1;

            //ISR execute
            execution += std:: to_string(current_time) + ", " + std:: to_string(ISR_var) + " call device driver " + std:: to_string(d) + "\n";
            current_time +=ISR_var;

            //IRET
            execution += std:: to_string(current_time) + ", 1 IRET\n";
            current_time +=1;

            //device finishing 
            device_finish_time[d] = -1;

        }else{
            //incase of an unkown activty
            execution += std:: to_string(current_time) + ", 0, unknown activity " + activity + "\n";
        }
        /************************************************************************/

    }

    input_file.close();

    write_output(execution);

    return 0;
}
