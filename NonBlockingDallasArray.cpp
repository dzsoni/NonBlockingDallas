#include "NonBlockingDallasArray.h"

NonBlockingDallasArray::NonBlockingDallasArray()
{
}

NonBlockingDallasArray::~NonBlockingDallasArray()
{
}


/**
 * Adds a NonBlockingDallas object to the NonBlockingDallasArray if it is not already present.
 *
 * @param NBDpt Pointer to the NonBlockingDallas object to be added.
 *
 * @throws None
 */
void NonBlockingDallasArray::addNonBlockingDallas(NonBlockingDallas *NBDpt)
{
    if (NBDpt == nullptr)
        return;
    // Don't add same pointer
    for (size_t i = 0; i < _wires.size(); i++)
    {
        if (_wires[i] == NBDpt) //same memory address?
            return;
        if (_wires[i]->getWireName() == NBDpt->getWireName()) //same wire name?
            return;
        if (_wires[i]->getGPIO() == NBDpt->getGPIO()) //same GPIO?
            return;
    }
    _wires.push_back(NBDpt);
    _wires.shrink_to_fit();
}

/**
 * Updates all wires in the NonBlockingDallasArray.
 *
 * @param None
 *
 * @return void
 *
 */
void NonBlockingDallasArray::update()
{
   for (size_t i = 0; i < _wires.size(); i++)
    {
        _wires[i]->update();
    } 
}

/**
 * Rescans all the wires in the NonBlockingDallasArray.
 *
 * @param None
 *
 * @return void
 */
void NonBlockingDallasArray::rescanWire()
{
    for (size_t i = 0; i < _wires.size(); i++)
    {
        _NBDARRAY_PL(String(__FUNCTION__) + ": Rescanning wire "+_wires[i]->getWireName());
        _wires[i]->rescanWire();
    }
}

/**
 * Request temperature for all sensors on all wires.
 *
 * @param None
 *
 * @return void

 */
void NonBlockingDallasArray::requestTemperature()
{
    for (size_t i = 0; i < _wires.size(); i++)
    {
        _wires[i]->requestTemperature();
    }
}


/**
 * Return the number of sensors on all wires.
 *
 * @return the total number of sensors
 */
const unsigned char NonBlockingDallasArray::getSensorsCount()
{
    unsigned char result=0;
    for (size_t i = 0; i < _wires.size(); i++)
    {
        result+=_wires[i]->getSensorsCount();
    }
    return result;
}

/**
 * Save sensor names to a file in JSON format.
 *
 * @return void
 *
 */
void NonBlockingDallasArray::saveSensorNames()
{
     if (_pathofsensornames != "")
    {
        String json = "{";
        for (unsigned int i = 0; i < _wires.size(); i++)
        {
            for (unsigned int e = 0; e < _wires[i]->getSensorsCount(); e++)
            {
                DeviceAddress address;
                ENUM_NBD_ERROR err;
                if ((e && i==0) || i>0 )
                    json += ",";
                json += "\"";
                _wires[e]->getAddressByIndex(e,address);
                json += _wires[e]->addressToString(address);
                json += "\":\"";
                json += _wires[e]->getSensorNameByIndex(e,err);
                json += "\"";
            }
        }
        json += "}";

        File file = SPIFFS.open(_pathofsensornames, "w");
        if (!file)
        {
            _DS18B20_PL(F("Error opening file for writing"));
            return;
        }
        _DS18B20_PL(json);
        file.print(json);
        file.flush();
        file.close();
    }
}

String NonBlockingDallasArray::addressToString(DeviceAddress sensorAddress)
{
    String str = "";
    for (int i = 0; i < 8; i++)
    {
        str += sensorAddress[i];
        if (i != 7)
            str += ".";
    }
    return str;
}



/**
 * Sets the resolution for the NonBlockingDallasArray object and propagates the resolution to all associated wires.
 * ('Wire' means a pin wich is used by NonBlockingDallas object)
 *
 * @param res the resolution to be set
 *
 * @return void
 *
 * @throws None
 */
void NonBlockingDallasArray::setResolution(NonBlockingDallas::NBD_resolution res)
{
    _res=res;
    for (size_t i = 0; i < _wires.size(); i++)
    {
        _wires[i]->setResolution(res);
    }
}

/**
 * Retrieves the resolution from the NonBlockingDallasArray.
 *
 * @return NBD_resolution the resolution value
 *
 */ 
NonBlockingDallas::NBD_resolution NonBlockingDallasArray::getResolution()
{
    return _res;
}

/**
 * Set the units of measure for the NonBlockingDallasArray.
 *
 * @param unit the units of measure to set
 *
 * @return void

 */
void NonBlockingDallasArray::setUnitsOfMeasure(NonBlockingDallas::NBD_unitsOfMeasure unit)
{
    _unitsOM=unit;
}

/**
 * Retrieves the units of measure for the NonBlockingDallasArray.
 *
 * @return NBD_unitsOfMeasure The units of measure
 */
NonBlockingDallas::NBD_unitsOfMeasure NonBlockingDallasArray::getUnitsOfMeasure()
{
    return _unitsOM;
}

/**
 * Returns the unit of measure as a string.
 *
 * @return "C" if the unit is Celsius, "F" if the unit is Fahrenheit
 */
String NonBlockingDallasArray::getUnitsOfMeasureAsString()
{
    return (_unitsOM==NonBlockingDallas::NBD_unitsOfMeasure::unit_C) ? "C" :"F";
}


/**
 * Retrieves the GPIO pin number for a sensor selected by its index.
 *
 * @param indexofsensor The index of the sensor.
 * @param err A reference to an ENUM_NBD_ERROR variable to store any errors that occur.
 *
 * @return The GPIO pin number of the sensor, or 0 if the index is out of range.
 *
 * @throws None
 */
unsigned char NonBlockingDallasArray::getGPIO(unsigned char indexofsensor,ENUM_NBD_ERROR &err)
{
    unsigned int pointer=0;
    for(unsigned int i=0; i<_wires.size();i++)
    {
      if(pointer+_wires.at(i)->getSensorsCount()>indexofsensor)
      {
        err=NBD_NO_ERROR;
        return _wires.at(i)->getGPIO();
      }
      else
      {
        pointer+=_wires.at(i)->getSensorsCount();
      }
    }
    err=NBD_INDEX_IS_OUT_OF_RANGE;
    return 0;
}

/**
 * Gets the name of the wire at the specified index if it exists.
 *
 * @param index the index of the wire to retrieve
 *
 * @return the name of the wire at the specified index, or an empty string if the index is out of bounds
 */
String NonBlockingDallasArray::getWireName(unsigned char index)
{
    if(index>=_wires.size())return "";
    return _wires[index]->getWireName();
}

/**
 * Sets the wire name for a specific index in the NonBlockingDallasArray.
 *
 * @param wirename the name to set for the wire
 * @param index the index of the wire to set the name for
 */
void NonBlockingDallasArray::setWireName(String wirename, unsigned char index)
{
    if(index>=_wires.size())return;
    _wires[index]->setWireName(wirename);
}

/**
 * Get temperature by index in the NonBlockingDallasArray.
 *
 * @param index the index of the temperature to retrieve
 * @param err reference to ENUM_NBD_ERROR to store error status
 *
 * @return the temperature value corresponding to the index
 *
 * @throws NBD_INDEX_IS_OUT_OF_RANGE if the index is out of range
 */
float NonBlockingDallasArray::getTempByIndex(unsigned char index, ENUM_NBD_ERROR &err)
{
    unsigned int pointer =0;
    err=NBD_NO_ERROR;
    for(unsigned int i=0; i<_wires.size();i++)
    {
      if(pointer+_wires.at(i)->getSensorsCount()>index)
      {
        return _wires.at(i)->getTempByIndex(index-pointer,err);
      }
      else
      {
          pointer+=_wires.at(i)->getSensorsCount();
      }
    }
    err = NBD_INDEX_IS_OUT_OF_RANGE;
    return (_unitsOM==NonBlockingDallas::NBD_unitsOfMeasure::unit_C) ? DEVICE_DISCONNECTED_C : DEVICE_DISCONNECTED_F;
}

/// @brief Returns the temperature value of the named sensor.
/// @param name name of the sensor
/// @param err NBD error
/// @return temp or DEVICE_DISCONNECTED_C OR DEVICE_DISCONNECTED_F
float NonBlockingDallasArray::getTempByName(String name, ENUM_NBD_ERROR &err)
{
    float temp=(_unitsOM==NonBlockingDallas::NBD_unitsOfMeasure::unit_C) ? DEVICE_DISCONNECTED_C : DEVICE_DISCONNECTED_F;
    
    ENUM_NBD_ERROR er;
    for(unsigned int i=0; i<_wires.size();i++)
    {
        er = NBD_NO_ERROR;
        temp=_wires.at(i)->getTempByName(name,er);
        if(er==NBD_NO_ERROR)
        {
            break;
        }
    }
    err=er;
    return temp;
}

/**
 * Retrieves the temperature by name.
 *
 * @param name the name of the temperature to retrieve
 *
 * @return the temperature value
 */
float NonBlockingDallasArray::getTempByNameS(String name)
{
    ENUM_NBD_ERROR ndb;
    return getTempByName(name,ndb);
}

/**
 * Get the sensor name by index in a NonBlockingDallasArray.
 *
 * @param index the index of the sensor
 * @param err reference to the error code
 *
 * @return the sensor name as a String
 *
 * @throws ENUM_NBD_ERROR if the index is out of range
 */
String NonBlockingDallasArray::getSensorNameByIndex(unsigned char index, ENUM_NBD_ERROR &err)
{
    unsigned int pointer =0;
    err=NBD_NO_ERROR;
    for(unsigned int i=0; i<_wires.size();i++)
    {
      if(pointer+_wires.at(i)->getSensorsCount()>index)
      {
        return _wires.at(i)->getSensorNameByIndex(index-pointer,err);
      }
      else
      {
          pointer+=_wires.at(i)->getSensorsCount();
      }
    }
    err = NBD_INDEX_IS_OUT_OF_RANGE;
    return String("");
}

/**
 * Set the sensor name by index in the NonBlockingDallasArray.
 *
 * @param index the index of the sensor
 * @param name the name to set for the sensor
 * @param err an ENUM_NBD_ERROR reference to store any error that occurs
 *
 * @return a boolean indicating if the operation was successful or not
 *
 */
bool NonBlockingDallasArray::setSensorNameByIndex(unsigned char index, String name, ENUM_NBD_ERROR &err)
{
    unsigned int pointer =0;
    err=NBD_NO_ERROR;
    for(unsigned int i=0; i<_wires.size();i++)
    {
      if(pointer+_wires.at(i)->getSensorsCount()>index)
      {
        return _wires.at(i)->setSensorNameByIndex(index-pointer,name,err);
      }
      else
      {
          pointer+=_wires.at(i)->getSensorsCount();
      }
    }
    err = NBD_INDEX_IS_OUT_OF_RANGE;
    return false;
}

/**
 * A function to get the index by sensor name in the NonBlockingDallasArray class.
 *
 * @param name the sensor name to search for
 * @param err an ENUM_NBD_ERROR reference to store any errors that occur
 *
 * @return an unsigned char representing the index found by sensor name
 *
 */
unsigned char NonBlockingDallasArray::getIndexBySensorName(String name, ENUM_NBD_ERROR &err)
{
    unsigned int pointer =0;
    err=NBD_NO_ERROR;
    for(unsigned int i=0; i<_wires.size();i++)
    {
      pointer+=_wires.at(i)->getIndexBySensorName(name,err);
      if(err==NBD_NO_ERROR)
      {
        return pointer;
      }
      pointer+=_wires.at(i)->getSensorsCount();
    }
    err=NBD_NAME_NOT_FOUND;
    return 0;
}


/**
 * Get the index by sensor name in the NonBlockingDallasArray.
 *
 * @param name the name of the sensor
 * @param index the index to be updated
 *
 * @return the error code ENUM_NBD_ERROR
 *  */
ENUM_NBD_ERROR NonBlockingDallasArray::getIndexBySensorName(String name, unsigned char &index)
{
    unsigned char pointer =0;
    ENUM_NBD_ERROR err = NBD_NO_ERROR;
    for(unsigned int i=0; i<_wires.size();i++)
    {
      pointer+=_wires.at(i)->getIndexBySensorName(name,err);
      if(err==NBD_NO_ERROR)
      {
        index=pointer;
        return err;
      }
      pointer+=_wires.at(i)->getSensorsCount();
    }
    index=0;
    err=NBD_NAME_NOT_FOUND;
    return err;
}

/**
 * A function to get the last time of valid temperature by index from a NonBlockingDallasArray.
 *
 * @param index the index to retrieve the last time of valid temperature
 * @param err an ENUM_NBD_ERROR reference to store any potential errors
 *
 * @return the last time of valid temperature associated with the given index
 *
 * @throws NBD_INDEX_IS_OUT_OF_RANGE if the index is out of range
 */
unsigned long NonBlockingDallasArray::getLastTimeOfValidTempByIndex(unsigned char index, ENUM_NBD_ERROR &err)
{
    unsigned int pointer =0;
    err=NBD_NO_ERROR;
    for(unsigned int i=0; i<_wires.size();i++)
    {
      if(pointer+_wires.at(i)->getSensorsCount()>index)
      {
        return _wires.at(i)->getLastTimeOfValidTempByIndex(index-pointer,err);
      }
      else
      {
          pointer+=_wires.at(i)->getSensorsCount();
      }
    }
    err = NBD_INDEX_IS_OUT_OF_RANGE;
    return 0;
}

/**
 * A function to get the last time of valid temperature by name from a NonBlockingDallasArray.
 *
 * @param name the name of the sensor to retrieve the last time of valid temperature for
 * @param err an ENUM_NBD_ERROR reference to store any potential errors
 *
 * @return the last time of valid temperature associated with the given sensor name
 *
 */
unsigned long NonBlockingDallasArray::getLastTimeOfValidTempByName(const String& name, ENUM_NBD_ERROR &err)
{
    ENUM_NBD_ERROR er;
    unsigned long temp=DEVICE_DISCONNECTED_C;
    for(unsigned int i=0; i<_wires.size();i++)
    {
        er = NBD_NO_ERROR;
        temp=_wires.at(i)->getLastTimeOfValidTempByName(name,er);
        if(er==NBD_NO_ERROR)
        {
            break;
        }
    }
    err=er;
    return temp;
}

/**
 * Get the address by index in the NonBlockingDallasArray.
 *
 * @param index the index of the address to retrieve
 * @param address reference to store the retrieved address
 *
 * @return an error code indicating the result of the operation
 */
ENUM_NBD_ERROR NonBlockingDallasArray::getAddressByIndex(unsigned char index, DeviceAddress &address)
{
    unsigned int pointer=0;
    for(unsigned int i=0; i<_wires.size();i++)
    {
      if(pointer+_wires.at(i)->getSensorsCount()>index)
      {
        return _wires.at(i)->getAddressByIndex(index-pointer,address);
      }
      else{
          pointer+=_wires.at(i)->getSensorsCount();
      }
    }
    return NBD_INDEX_IS_OUT_OF_RANGE;
}

String NonBlockingDallasArray::getAddressByIndexS(unsigned char index)
{
    DeviceAddress address;
    unsigned int pointer=0;
    for(unsigned int i=0; i<_wires.size();i++)
    {
      if(pointer+_wires.at(i)->getSensorsCount()>index)
      {
        _wires.at(i)->getAddressByIndex(index-pointer,address);
        return addressToString(address);
      }
      else{
          pointer+=_wires.at(i)->getSensorsCount();
      }
    }
    return String();
}

/**
 * Set the sensor name by address in the NonBlockingDallasArray.
 *
 * @param addr the address of the sensor
 * @param name the name to set for the sensor
 * @param err an ENUM_NBD_ERROR reference to store any error that occurs
 *
 * @return a boolean indicating if the operation was successful or not
 *
 */
bool NonBlockingDallasArray::setSensorNameByAddress(const DeviceAddress addr, String name, ENUM_NBD_ERROR &err)
{
    err=NBD_NO_ERROR;
    for(unsigned int i=0; i<_wires.size();i++)
    {
      if(_wires[i]->setSensorNameByAddress(addr,name,err))
      {
        return true;
      }
    }
    err = NBD_ADDRESS_IS_NOT_FOUND;
    return false;
}

/**
 * Retrieves the name of a sensor in the NonBlockingDallasArray based on its address.
 *
 * @param addr The address of the sensor.
 * @param err A reference to an ENUM_NBD_ERROR to store any error that occurs.
 *
 * @return The name of the sensor if found, otherwise an empty string.
 *
 * @throws None.
 */
String NonBlockingDallasArray::getSensorNameByAddress(const DeviceAddress addr, ENUM_NBD_ERROR &err)
{
    err=NBD_NO_ERROR;
    DeviceAddress addr2;
    for(unsigned int i=0; i<getSensorsCount();i++)
    {
        if(getAddressByIndex(i,addr2)==NBD_NO_ERROR)
        {
            if(addr2[0]==addr[0] && addr2[1]==addr[1] && addr2[2]==addr[2] && addr2[3]==addr[3] && addr2[4]==addr[4] && addr2[5]==addr[5] && addr2[6]==addr[6] && addr2[7]==addr[7])
            {
                return getSensorNameByIndex((unsigned char)i,err);
            }
        }       
    }
    err = NBD_ADDRESS_IS_NOT_FOUND;
    return String();
}

String NonBlockingDallasArray::getSensorNameByAddressS(const DeviceAddress addr)
{
    ENUM_NBD_ERROR err;
    DeviceAddress addr2;
    for(unsigned int i=0; i<getSensorsCount();i++)
    {
        if(getAddressByIndex(i,addr2)==NBD_NO_ERROR)
        {
            if(addr2[0]==addr[0] && addr2[1]==addr[1] && addr2[2]==addr[2] && addr2[3]==addr[3] && addr2[4]==addr[4] && addr2[5]==addr[5] && addr2[6]==addr[6] && addr2[7]==addr[7])
            {
                return getSensorNameByIndex((unsigned char)i,err);
            }
        }       
    }
    return String();
}

/**
 * Init all devices with same parameter.
 *
 * @param res resolution
 * @param uom unitOfMeasure
 * @param tempInterval Interval among each sensor reading [milliseconds]
 *
 */
void NonBlockingDallasArray::begin(NonBlockingDallas::NBD_resolution res, NonBlockingDallas::NBD_unitsOfMeasure uom, unsigned long tempInterval)
{
    _res=res;
    _unitsOM=uom;
    for (size_t i = 0; i < _wires.size(); i++)
    {
        _wires[i]->setPathOfSensorNames(_pathofsensornames);
        _wires[i]->begin(res,uom,tempInterval);
    }
}

void NonBlockingDallasArray::begin(NonBlockingDallas::NBD_resolution res, NonBlockingDallas::NBD_unitsOfMeasure uom, unsigned long tempInterval, String pathofsensornames)
{
    _pathofsensornames=pathofsensornames;
    begin(res,uom,tempInterval);
}
