#ifndef __CheckSum_h__
#define __CheckSum_h__


#include "zlib.h"

template <typename T>
class CheckSum{
public:
  CheckSum() :
    sum( T::update( 0L, Z_NULL, 0 ) ){};
  CheckSum& add( const Bytef *buf, uInt len ){
    sum = T::update( sum, buf, len );
    return *this;
  };
  CheckSum& reset(){
    sum = T::update( 0L, Z_NULL, 0 );
    return *this;
  };
  uLong get() const {
    return sum;
  };
private:
  uLong sum;
};

class Adler32{
public:
  static uLong update( uLong adler, const Bytef *buf, uInt len ){
    return adler32(adler, buf, len);
  }
};

class CRC32{
public:
  static uLong update( uLong crc, const Bytef *buf, uInt len ){
    return crc32(crc, buf, len);
  }
};

#endif
