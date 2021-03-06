/**
 * Meter Main Application File:  MeterMain.cpp
 *
 * This file is the main source code file for the client application.
 * It represents the Utility Meter reader and uses various classes to
 * store data.  It utilizes the OneM2M open standards to communicate with
 * the server.
 *
 * Instructions:
 *     See docs/client.md for the instructions on how to compile and execute
 *         this client application.
 *
 * @author Team 6
 *
 * @version 1.0
 */

// Imports
#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <ctime>
#include "onem2m.hxx"
#include "UtilityMeter.h"

using namespace std;

/**
  * Function declarations.
  */
bool isValidIP (char x[]);

/**
  * Main function, program entry point.
  */
int main (int argc, char* argv[]) {

  // Global Function Variables
  long result;                          // HTTP Result code.
  string hostName = "127.0.0.1:8080";   // The IP:Port of OM2M server.
  string loginCred = "admin:admin";     // The OM2M Server login credentials.
  string aeName = "MY_METER";           // Name of the AE Resource to create.
  string aeAppId = "app1";              // Name of the AE App Id. Mandatory.
  string contName = "DATA";             // Data Container Name.
  ::xml_schema::integer respObjType;    // The response data from server.
  string cseRootAddr = "/in-cse/in-name";            // SP-Relative address.
  std::unique_ptr< ::xml_schema::type > respObj;     // The result code from server.
  UtilityMeter um;                      // Construct our UtilityMeter object.
  int meterValue;                       // Represents Utility Meter Value.
  string meterValueStr;                 // Utility Meter Value as a string.
  string buff= "type = Utility_Meter\n"           //String for UtilityMeter description.
    "location = Home\n"
    "appIDd = MY_METER";
  um.setMeterDescriptor(buff);          // Set the Descriptor for the UtilityMeter object.
  double secondsPassed;
  double secondsToDelay = 10;           // Seconds between meter-value updates
  int count = 0;                        // Test value counter
  int runtime = 0;                      // Runtime value in minutes
  int countCalc = 13;                   // To be calculated using 60 * runtime / secondsToDelay + 1 , Default to 13 for 2 minutes
  double time_counter = 0;              // Timer for simulated data
  clock_t this_time = clock();
  clock_t last_time = this_time;

  /*
   * Parse command line args
   */
  if (argc == 1) {   // Arg count 1 is just the program name.
    cout << "\nNo command line args passed...\n";
  }
  if (argc >= 2) {   // Arg count 2 is the OM2M server in format IP:Port
    cout << "\nCommand line arg passed for OM2M server: ";
    cout << argv[1] << endl;
    // Validate the argument is in IP:Port format.
    if (isValidIP(argv[1])) {
      hostName = argv[1];    // Set hostName to command line arg IP:Port
    }
    else {
      cout << "Invalid argument for OM2M IP:Port - " << argv[1]
      << "\n   Exiting...\n";
      return 0;
    }
  }
  if (argc >= 3) { // Arg count 3 is the desired run-time in minutes
        cout << "\nCommand line arg passed for run-time in minutes: ";
        cout << argv[2] << endl;
        runtime = atoi(argv[2]);
        countCalc = 60 * runtime / secondsToDelay + 1;
  }

  /*
   * First, initialize the OS-IoT library.
   */
  cout << "\nInitializing oneM2M library...";
  onem2m::initialize();
  cout << "Done!\n";

  /*
   * Update the values for pre-set parameters.
   */
  cout << "Updating pre-set parameters...";
  onem2m::setHostName(hostName);    // OM2M server address.
  onem2m::setFrom(loginCred);       // Credentials.
  cout << "Done!\n";

  /*
   * Create the main MY_METER AE resource object and push to OM2M server.
   */
  cout << "Creating MY_METER AE resource object...\n";
  auto aeMyMeter = onem2m::AE();
  aeMyMeter.resourceName(aeName);
  aeMyMeter.App_ID(aeAppId);
  aeMyMeter.requestReachability(false); // requestReachability is mandatory for AE.
  respObj = onem2m::createResource(cseRootAddr, "5555", aeMyMeter,
    result, respObjType);
  cout << "Result code is:  " << result << "\n";

  /*
   * Retrieve the AE resource.  For testing.
   */
  cout << "\nRetrieving AE resource object...\n";
  respObj = ::onem2m::retrieveResource(cseRootAddr+"/"+aeName,
    "5555", result, respObjType);
  cout << "Result = " << result << "\n";
  cout << "respObjType = " << respObjType << "\n";

  /*
   * Create a container in our AE. This container will store the AE description.
   */
   cout << "\nCreating Descriptor Container...\n";
  auto dsccnt = ::onem2m::container();
  dsccnt.resourceName("DESCRIPTOR");
  respObj = ::onem2m::createResource(cseRootAddr+"/"+aeName,
    "5555", dsccnt, result, respObjType);
  cout << "\nContainer creation result code: " << result << "\n";

  /*
   * Populate the descriptor container with an actual discription
   */
  cout << "\nCreating Descriptor Content Instance...\n";
  auto descInst = ::onem2m::contentInstance();
  descInst.contentInfo("application/text");      // Text data.
  descInst.content(um.getMeterDescriptor());
  respObj = ::onem2m::createResource(cseRootAddr+"/"+aeName+"/"+"DESCRIPTOR",
    "5555", descInst, result, respObjType);

  /*
   * Create a Container in our AE. This container will store the meter value.
   */
  cout << "\nCreating Container...\n";
  auto cnt = ::onem2m::container();
  cnt.resourceName(contName);
  respObj = ::onem2m::createResource(cseRootAddr+"/"+aeName,
    "5555", cnt, result, respObjType);
  cout << "\nContainer creation result code: " << result << "\n";

  /*
   * Write simulated utility meter value data to Content Instance in the Container
   */
  cout << "\nCreating Content Instance...\n";
  auto contInst = ::onem2m::contentInstance();
  contInst.contentInfo("application/text");      // Text data.
  meterValue = um.getRandomValue();              // Get a simulated value from meter.
  meterValueStr = "{\"kWH\": " + to_string(meterValue) + "}";   // Convert int to Str.
  contInst.content(meterValueStr);               // Write simulated value.
  respObj = ::onem2m::createResource(cseRootAddr+"/"+aeName+"/"+contName,
    "5555", contInst, result, respObjType);

  /*
   * Get the Content Instance resource. For testing.
   */
  cout << "\nRetrieving Content Instance resource object...\n";
  respObj = ::onem2m::retrieveResource(cseRootAddr+"/"+aeName+"/"+contName,
    "5555", result, respObjType);
  cout << "Result = " << result << "\n";
  cout << "respObjType = " << respObjType << "\n";

  /*
   * Update and display the meter-value every 10 seconds for webapp
   * testing (based on secondsToDelay).
   */
  cout << "Meter values will now update every " << secondsToDelay;
  cout << " seconds for " << runtime;
  cout << " minutes...\n";

    while(count < countCalc){
        this_time = clock();
        time_counter += (double)(this_time - last_time);
        last_time = this_time;

        if(time_counter > (double)(secondsToDelay * CLOCKS_PER_SEC))
        {
            time_counter -= (double)(secondsToDelay * CLOCKS_PER_SEC);

            um.updateMeterValueRand(); //update meterValue with random number.
            meterValue = um.getMeterValue();
            cout << "meter-value: " << meterValue << endl;
            // create a new contentInstance
            contInst = ::onem2m::contentInstance();
            contInst.contentInfo("application/text");      // Text data.
            meterValueStr = "{\"kWH\": " + to_string(meterValue) + "}"; // Convert int to Str.
            contInst.content(meterValueStr);               // Write simulated value.
            respObj = ::onem2m::createResource(cseRootAddr+"/"+aeName+"/"+contName,
              "5555", contInst, result, respObjType);

            count++;
        }
    }

  /*
   * Close the OS-IoT library and exit.
   */
  ::onem2m::terminate();
  cout << "Finished!\n";

  return 1;

} // End of main




/**
  * This function checks to ensure that the provided char
  * array is in a valid IP:Port-number format.  For example,
  * 127.0.0.1:8080 is a valid IP:Port-number formatted char
  * array.
  *
  * @param The input char array to validate.
  * @return Boolean indicating whether input is valid or not.
  */
bool isValidIP(char x[]) {

  // Initialize the result boolean to true.
  bool result = true;

  int c1 = 0;     // Count number of "." characters.
  int c2 = 0;     // Count number of ":" characters.

  // Iterate through the input char array.
  int i = 0;
  while (x[i] != '\0') {

    // If the character matches "." then increment the relevant counter.
    if (x[i] == '.') {
      c1++;
    }

    // If the character matches ":" then increment the relevant counter.
    else if (x[i] == ':') {
      c2++;
    }

    i++;
  } // End of while loop.

  // If "." characters do not equal 3, result is false.
  if (c1 != 3) {
    result = false;
  }
  // If ":" characters do not equal 1, result is false.
  else if (c2 != 1) {
    result = false;
  }

  // Return the result back.
  return result;

} // End of function isValidIP.
