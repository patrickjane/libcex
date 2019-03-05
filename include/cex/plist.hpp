//*************************************************************************
// File plist.hpp
// Date 14.05.2018 - #1
// Copyright (c) 2018-2018 by Patrick Fial
//-------------------------------------------------------------------------
// Class PropertyList
//*************************************************************************

#ifndef __PLIST_HPP__
#define __PLIST_HPP__

/*! \file plist.hpp 
  \brief Implementation of a simple propertylist based on `std::unordered_map`
*/


//***************************************************************************
// includes
//***************************************************************************

#include <unordered_map>
#include <string>

namespace cex
{

//***************************************************************************
// class Property
//***************************************************************************
/*! \class Property
  \brief Describes a single property containing one or more typed values

  A property can have several values, one for each of the types (string, long, double, void*)
  */

class Property
{
   public:

      /*! \brief Constructs a new property with a string value */
      explicit Property(const std::string& value) : stringValue(value), longValue(0), doubleValue(0), ptrValue(0) {}
      /*! \brief Constructs a new property with a string value (moving the value) */
      explicit Property(std::string&& value)      : stringValue(value), longValue(0), doubleValue(0), ptrValue(0) {}
      /*! \brief Constructs a new property with a long value */
      explicit Property(long value)               : longValue(value), doubleValue(0), ptrValue(0) {}
      /*! \brief Constructs a new property with a double value */
      explicit Property(double value)             : longValue(0), doubleValue(value), ptrValue(0) {}
      /*! \brief Constructs a new property with a void* value */
      explicit Property(void* value)              : longValue(0), doubleValue(0), ptrValue(value) {}

      /*! \brief Retrieves the string value of the property. If no string value was set, returns an empty string object */
      std::string& getStringValue()  { return stringValue; }
      /*! \brief Retrieves the long value of the property. If no long value was set, returns 0 */
      long getLongValue()            { return longValue; }
      /*! \brief Retrieves the double value of the property. If no double value was set, returns 0 */
      double getDoubleValue()        { return doubleValue; }
      /*! \brief Retrieves the void* value casted to the template type. If no void* was set, returns a null-pointer */
      template<typename T> T* getObjectValue() { return  (T*)ptrValue; }

   private:

      std::string stringValue;
      long longValue;
      double doubleValue;
      void* ptrValue;
};

//***************************************************************************
// class PropertyList
//***************************************************************************
/*! \class PropertyList
  \brief A simple list of properties implemented using `std::unordered_map`

  Each entry in the map is of type Property and can hold a value*/

class PropertyList
{
   public:

      /*! \brief Retrieves the Property object of a given key */
      Property* getProperty(std::string key) 
      {
         std::shared_ptr<Property> res= entries[key];
         return res ? res.get() : nullptr;
      }
      
      /*! \brief Retrieves the value of a given key as a class-pointer value (of type `T`) */
      template<typename T> 
      T* getObject(std::string key) 
      { 
         std::shared_ptr<Property> res= entries[key];
         return res ? res.get()->getObjectValue<T>() : nullptr;
      }

      /*! \brief Retrieves the long value of a given key */
      long getLong(std::string key)
      {
         std::shared_ptr<Property> res= entries[key];
         return res ? res.get()->getLongValue() : 0;
      }
      
      /*! \brief Retrieves the double value of a given key */
      double getDouble(std::string key)
      {
         std::shared_ptr<Property> res= entries[key];
         return res ? res.get()->getDoubleValue() : 0;
      }

      /*! \brief Retrieves the string value of a given key */
      std::string getString(std::string key)
      {
         std::shared_ptr<Property> res= entries[key];
         return res ? res.get()->getStringValue() : std::string();
      }

      /*! \brief Sets the value of a key to a string value. Replaces previous values of a key */
      void set(std::string key, std::string value)   { entries[key]= std::make_shared<Property>(value); }
      /*! \brief Sets the value of a key to a string value (moving the content). Replaces previous values of a key */
      void set(std::string key, std::string&& value) { entries[key]= std::make_shared<Property>(value); }
      /*! \brief Sets the value of a key to a long value. Replaces previous values of a key */
      void set(std::string key, long value)          { entries[key]= std::make_shared<Property>(value); }
      /*! \brief Sets the value of a key to a double value. Replaces previous values of a key */
      void set(std::string key, double value)        { entries[key]= std::make_shared<Property>(value); }
      /*! \brief Sets the value of a key to a void* value. Replaces previous values of a key */
      void set(std::string key, void* value)         { entries[key]= std::make_shared<Property>(value); }

      /*! \brief Checks if the list contains a given key */
      bool has(std::string key)    { return entries.count(key) > 0; }

      /*! \brief Removes a key from the list and returns the number of elements removed (0 or 1) */
      size_t remove(std::string key) { return entries.erase(key); }

   private:

      std::unordered_map<std::string, std::shared_ptr<Property>> entries;
};

//***************************************************************************
} // namespace cex

#endif // __PLIST_HPP_
