
/*  vendingmachine.ino
 *  Copyright David Denney 2014
 *
 *  Released publicly under the GPLv2 license.  
 *
 *  Code for controlling the outputs on our vending machine.
 *  Supports vending, associating an output pin with a given designation (usually indicating the slot on the machine).
 *  Will eventually support status query response, allowing the connected system to query for what slots are out of stock (useful if this is not tracked on the managing system.
 *
 */

/*
 *  Current Issue: Will properly handle a single vend by label command.
 *  Subsequent commands may get hung up.
 *  No other commands tested at this time.
 *
 * (Mostly committed this so the git repository's readme wouldn't ge lonely.)
 */
 
 
 /*
   Protocol for incoming messages:
     CmdType Arg1 Arg2 ... crlf
     
     Where CmdType is a single byte determining the command type.
     Arguments are delimited from the CmdType and each other by single space characters.
     The 'crlf' bytes (0x0d 0x0a) indicates message end.
    
     Arguments that are numerical will be read as  single byte, followed by the delimiter character.  Therefore, if Arg1 is the number 15, it will be represented by the byte 0x0f.
     Arguments that are strings (should only be for output labels), are in ASCII.  
     A valid command must therefore be at least 5 bytes long (CmdType, space, Arg, cr, lf).
     
    
     CmdType V - Vend.
      Arg1: L - Label, P - Pin.
      
      Arg2: A null terminated string for label, or a single byte for an output pin.
     
     
     CmdType S - SetLabel.
      Arg1: Pin, a byte indicating which output pin is associated with the given pin.
      If a current pin already has the given label, that association will be removed and the new one added.
      If the second argument is 255 (0xff), the given label will be removed from association without replacement. 
      
      Arg2: Label, as a null terminated string designating the 'handle' for a given output pin.
       
 */
 
const byte _lf    = 0x0a;  //11=line feed
const byte _cr    = 0x0d; //13 = carriage return;
const byte _space = 0x32; //don't remember at the moment.
const int _bufflen = 10; //length of the input buffer.  10 seems like a good number, this can be changed if desired.

const int _labelLen=54;
String* _labels[_labelLen]; //54 outputs on an Arduino Mega 2560, ymmv.



void setup()
{
  //Setup the initial label system here as desired.  That way this can be handled onboard, in a static way, not requiring the calling system to know
  //what hardware pins are bound to which labels.  
  _labels[0]=new String("A");
  _labels[1]=new String("B");
  _labels[2]=new String("C");
  _labels[3]=new String("D");
  _labels[4]=new String("E");
  _labels[5]=new String("F");
  _labels[6]=new String("G");
  _labels[7]=new String("H");
  _labels[8]=new String("I");
  _labels[9]=new String("J");
  _labels[10]=new String("K");
  _labels[11]=new String("L");
  _labels[12]=new String("M");
  _labels[13]=new String("N");
  _labels[14]=new String("O");
  _labels[15]=new String("P");
  _labels[16]=new String("R");//there's no Q on the machine
  _labels[17]=new String("S");
  _labels[18]=new String("T");
  _labels[19]=new String("U");
  _labels[20]=new String("V");
  _labels[21]=new String("W");
  _labels[22]=new String("X");
  _labels[23]=new String("Y");
  _labels[24]=new String("Z");
  _labels[25]=new String("AA");
  _labels[26]=new String("BB");
  _labels[27]=new String("CC");
  _labels[28]=new String("DD");
  _labels[29]=new String("EE");
  _labels[30]=new String("G1");//Gum 1
  _labels[31]=new String("G2");
  _labels[32]=new String("G3");
  _labels[33]=new String("G4");
  _labels[34]=new String("G5");
  _labels[35]=new String("A1");//Accessories
  _labels[36]=new String("A2");
  _labels[37]=new String("A3");
  _labels[38]=new String("A4");
  _labels[39]=new String("A5");
  _labels[40]=new String("A6");
  _labels[41]=new String("A7"); //'(' maps to ascii 41 
  _labels[42]=new String("A8");
  _labels[43]=new String("A9");
  _labels[44]=new String("A10");
  _labels[45]=new String("A11");
  _labels[46]=new String("A12");
  _labels[47]=new String("A13");
  _labels[48]=new String("A14");
  _labels[49]=new String("A15");
  _labels[50]=new String("A16");
  _labels[51]=new String("A17");
  _labels[52]=new String("A18");
  _labels[53]=new String("A19");
  
  Serial.setTimeout(5000);//5 seconds.
  Serial.begin(9600);
}

void loop()
{
  //Serial.println("Starting listen for Serial.available()");
  if (Serial.available() > 4)
  {
    char _buff[_bufflen];
    Serial.println("Starting readUntil");
    int bytesRead = Serial.readBytesUntil(_lf, _buff, _bufflen);//read up to _bufflen chars, or until a line return is seen
    Serial.println("Exited readUntil");
    
    if(bytesRead < 1)//nothing read (timeout) or read error, loop around and try again. Should never happen, since we loop until there is something available.
    {
      Serial.println("Error 1");
      return;
    }
      
    if(bytesRead < 5)
    {
      Serial.println("Received too few bytes, which is too few to be valid.");
      Serial.flush();
      return; 
    }
    
    //Serial.println("Msg len: "+bytesRead);
    Serial.println("printing buff");
    
    for(int i=0;i<bytesRead;i++)
    {
      Serial.print(" 0x");
      Serial.print(_buff[i],HEX);
    }
    Serial.println("");
    
    if(_buff[bytesRead-1]!=_cr)
    {
      Serial.println("Message did not terminate with a CRLF");
      return; 
    }
    
    Serial.println("3");
    
    _buff[bytesRead-1]=0x00;//null, ends the string

    Serial.println("4");

    if(IsVendRequest(_buff))
    {
      Serial.println("Is a vend request");
      HandleVendRequest(_buff + 2); //start at the first argument, and make it clear that 
    }
    else if(IsSetLabelRequest(_buff))
    {
      Serial.println("Label request");
      HandleSetLabelRequest(_buff + 2);
    }    
    
    Serial.println("Finishing loop");
    //Should maybe make this return a status value, to let the other endpoint know whether the command was accepted or not.
    //Maybe error code, space, descriptive string, crlf.
    delete(_buff); //the only memory ever used excepting new labels being created.
    
    Serial.println("Done deleting");
  }
}

bool HandleSetLabelRequest(char buff[])
{
  Serial.println("In HandleSetLabelRequest");
  //first byte is the pin number
  byte pinByte = (byte)buff[0];
  Serial.println("Got byte");
  int pinNum = (int)pinByte;
  Serial.println("Got pin for setlabel " + String(pinNum));
  
  //then a space (  buff[1]  )
  
  //then a string designating the new label for that pin.
  String labelFromBuff = (String)buff[2]; //buff is going to get memory collected at the end of this loop(), so copy this to a new string.
  Serial.println("Got new label "+labelFromBuff+" for pin "+String(pinNum));
  String* newLabel = new String(labelFromBuff); //create a new string, get the pointer to it
  Serial.println(*newLabel+" is the new copy");

  //delete existing string at that location if it exists
  if(_labels[pinNum])
  {
    Serial.println("Deleting existing entry for "+String(pinNum)+" which was "+*(_labels[pinNum]));
    delete(_labels[pinNum]);
    _labels[pinNum] = NULL;
  }
 
  Serial.println("Setting new label");
  //put new string into the array
  _labels[pinNum] = newLabel;
}

bool IsVendRequest(char buff[])
{
  if(buff[0] == 'V')
    return true;
  return false;
}

bool IsSetLabelRequest(char buff[])
{
  if(buff[0] == 'S')
    return true;
  return false; 
}

void HandleVendRequest(char buff[])
{ 
   Serial.println("In HandleVendRequest");
   if(buff[0] == 'L') //next arg is a string, vend by label
   {
     //last character before the carriage return must be a null to terminate the cstring.
     Serial.println("Label vend request");
     //Serial.println("- Next char "+buff[2]);
     VendByLabel(buff + 2);
   }
   else if(buff[0] == 'P')
   {
     Serial.println("Pin vend request");
     VendByPin(buff + 2);
   }
} 

//DaVinci's edit.
//AQ
//..ait a quarter second, then deactivate it, and return. (Simulate button press).

void VendByPin(char buff[])
{
   //read the pin as a byte, set that pin as an output, activate it, wait a quarter second, then deactivate it, and return. (Simulate button press).
   Serial.println("Vending by pin: ");
//   Serial.print(" 0x");
//   Serial.print((byte)buff[0],HEX);
 
   pinMode((byte)buff[0], OUTPUT);
   Serial.println("Pin High");
   digitalWrite((byte)buff[0], HIGH);
   delay(2500);
   digitalWrite((byte)buff[0], LOW);
   Serial.println("Pin Low");
   
}

void VendByLabel(String buff)
{
  Serial.println("In VendByLabel");
  
  Serial.println("printing buff");
    
//    for(int i=0;i<2;i++)
//    {
//      Serial.print(" 0x");
//      Serial.print(buff[i],HEX);
//    }
//    Serial.println("");
    
  
  //Serial.println("Printing string \""+buff+"\"");
  //only works if the string is null terminated, as it should be-but we haven't added error checking for that...
  
  //find a string in the labels collection that has an associated pin number.

  String totest=buff;
  int pinToVend = GetPinForLabel(totest);
  if(pinToVend==-1)
  {
    Serial.println("Pin to vend for was not found in GetPinForLabel.");
    return;
  }
  char arr[]{(byte)pinToVend};
  VendByPin(arr);
}

int GetPinForLabel(String totest)
{
  Serial.println("In GetPinForLabel");
  for(int i=0;i<_labelLen;i++)
  {
    Serial.println("Testing "+*_labels[i]);
    if(_labels[i] && totest==*(_labels[i]))
    {
      Serial.println("Found pin "+String(i));
      int pinForLabel = i;
      return pinForLabel;
    }
  } 
  return -1;
}

