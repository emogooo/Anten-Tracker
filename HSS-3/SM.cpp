#include "SM.h"

SM::SM(byte SMXStepPin, byte SMXDirPin, byte SMYStepPin, byte SMYDirPin, byte SMX350DereceLimitPin, byte SMX0DereceLimitPin, byte SMY90DereceLimitPin, byte SMY0DereceLimitPin){
  _SMXStepPin = SMXStepPin;
  _SMXDirPin = SMXDirPin;
  _SMYStepPin = SMYStepPin;
  _SMYDirPin = SMYDirPin;
  _SMX350DereceLimitPin = SMX350DereceLimitPin;
  _SMX0DereceLimitPin = SMX0DereceLimitPin;
  _SMY90DereceLimitPin = SMY90DereceLimitPin;
  _SMY0DereceLimitPin = SMY0DereceLimitPin;
  hizAyarla(300,300); // Kalibrasyondan sonraki hareket hızı
}

void SM::_xBirAdimAt(){
  digitalWrite(_SMXStepPin, HIGH);
  delayMicroseconds(_xHiz);
  digitalWrite(_SMXStepPin, LOW);
  delayMicroseconds(_xHiz);
}

void SM::_yBirAdimAt(){
  digitalWrite(_SMYStepPin, HIGH);
  delayMicroseconds(_yHiz);
  digitalWrite(_SMYStepPin, LOW);
  delayMicroseconds(_yHiz);
}

void SM::SMXKalibrasyon(){
  _xHiz = 400; // Kalibrasyon hızı
  _xYonAyarla(HIGH);
  _xEkseniAdimSayisi = 0;
  while(digitalRead(_SMX350DereceLimitPin) == LOW){
    _xBirAdimAt();
  }
  _xYonAyarla(!_SMXDir);
  while(digitalRead(_SMX0DereceLimitPin) == LOW){
    _xBirAdimAt();
    _xEkseniAdimSayisi++;
  }
  _xYonAyarla(!_SMXDir);
  for(int i = 0; i < 500; i++){                     // Limit Butona baskı yapmaması için 500 adım geri döner. Burada ki 500 adım kör noktadır. Adım sayısı gerektiğinde yalnızca içeriden değiştirilebilir.
    _xBirAdimAt();
  }
  _xEkseni1DereceIcinAdimSayisi = (_xEkseniAdimSayisi - 1000) / 350.0;
  _xKonumDerecesi = 0;
}

void SM::SMYKalibrasyon(){
  _yHiz = 200; // Kalibrasyon hızı
  _yYonAyarla(HIGH);
  _yEkseniAdimSayisi = 0;
  while(digitalRead(_SMY90DereceLimitPin) == LOW){
    _yBirAdimAt();
  }
  _yYonAyarla(!_SMYDir);
  while(digitalRead(_SMY0DereceLimitPin) == LOW){
    _yBirAdimAt();
    _yEkseniAdimSayisi++;
  }
  _yYonAyarla(!_SMYDir);
  for(int i = 0; i < 1000; i++){                     // Limit Butona baskı yapmaması için 1000 adım geri döner. Burada ki 1000 adım kör noktadır. Adım sayısı gerektiğinde yalnızca içeriden değiştirilebilir.
    _yBirAdimAt();
  }
  _yEkseni1DereceIcinAdimSayisi = (_yEkseniAdimSayisi - 2000) / 90.0;
  _yKonumDerecesi = 0;
}

void SM::_xYonAyarla(byte yon){
  _SMXDir = yon;
  digitalWrite(_SMXDirPin, _SMXDir);
}

void SM::_yYonAyarla(byte yon){
  _SMYDir = yon;
  digitalWrite(_SMYDirPin, _SMYDir);
}

void SM::_xTekilHareket(unsigned int gidilecekKonumunDerecesi){
  // Yönün belirlenmesi.
  if(_xKonumDerecesi < gidilecekKonumunDerecesi){ // 0dan 100e
    _xYonAyarla(HIGH);
  }else{                                       // 100den 0a
    _xYonAyarla(LOW);
  }
  // Kaç derecelik açı yapacağının belirlenmesi.
  _xGidilecekDereceSayisi = abs(_xKonumDerecesi - gidilecekKonumunDerecesi);
  // Atılacak adımın belirlenmesi.
  _xGidilecekAdimSayisi = _xGidilecekDereceSayisi * _xEkseni1DereceIcinAdimSayisi;
  // Hızların belirlenmesi.
  if(_xGidilecekDereceSayisi < 8){  // Kısa mesafe hareket 1/32 için minimum 56 adım. 20-20 ivme adım payı.
    _xHareketIvmeAdimPayi = 20;
    _xHareketIvmeAdimBasinaDegisecekHiz = 10;
    _xHiz = _xHizTutucu + 400;  
  }else{  // Uzun mesafe hareket 1/32 için minimum 448 adım. 200-200 ivme adım payı.
    _xHareketIvmeAdimPayi = 200;
    _xHareketIvmeAdimBasinaDegisecekHiz = 1;
    _xHiz = _xHizTutucu;
  }
  _xGidilecekAdimSayisi = _xGidilecekAdimSayisi - (_xHareketIvmeAdimPayi * 2);
  // Hareket fonksiyonu.
  for(unsigned int i = 0; i < _xHareketIvmeAdimPayi; i++){
    _xBirAdimAt();
    _xHiz = _xHiz - _xHareketIvmeAdimBasinaDegisecekHiz;
  }
  for(unsigned int i = 0; i < _xGidilecekAdimSayisi; i++){
    _xBirAdimAt();
  }
  for(unsigned int i = 0; i < _xHareketIvmeAdimPayi; i++){
    _xBirAdimAt();
    _xHiz = _xHiz + _xHareketIvmeAdimBasinaDegisecekHiz;
  }  
  // Konum güncelleme.
  _xKonumDerecesi = gidilecekKonumunDerecesi;
}

void SM::_yTekilHareket(unsigned int gidilecekKonumunDerecesi){
  // Yönün belirlenmesi.
  if(_yKonumDerecesi < gidilecekKonumunDerecesi){ // 0dan 100e
    _yYonAyarla(HIGH);
  }else{                                       // 100den 0a
    _yYonAyarla(LOW);
  }
  // Kaç derecelik hareket yapacağının belirlenmesi.
  _yGidilecekDereceSayisi = abs(_yKonumDerecesi - gidilecekKonumunDerecesi);
  // Atılacak adımın belirlenmesi.
  _yGidilecekAdimSayisi = _yGidilecekDereceSayisi * _yEkseni1DereceIcinAdimSayisi;
  // Hızların belirlenmesi.
  if(_yGidilecekDereceSayisi < 8){  // Kısa mesafe hareket 1/32 için minimum 56 adım. 20-20 ivme adım payı.
    _yHareketIvmeAdimPayi = 20;
    _yHareketIvmeAdimBasinaDegisecekHiz = 10;
    _yHiz = _yHizTutucu + 400;
  }else{  // Uzun mesafe hareket 1/32 için minimum 448 adım. 200-200 ivme adım payı.
    _yHareketIvmeAdimPayi = 200;
    _yHareketIvmeAdimBasinaDegisecekHiz = 1;
    _yHiz = _yHizTutucu;
  }
  _yGidilecekAdimSayisi = _yGidilecekAdimSayisi - (_yHareketIvmeAdimPayi * 2);
  // Hareket fonksiyonu.
  for(unsigned int i = 0; i < _yHareketIvmeAdimPayi; i++){
    _yBirAdimAt();
    _yHiz = _yHiz - _yHareketIvmeAdimBasinaDegisecekHiz;
  }
  for(unsigned int i = 0; i < _yGidilecekAdimSayisi; i++){
    _yBirAdimAt();
  }
  for(unsigned int i = 0; i < _yHareketIvmeAdimPayi; i++){
    _yBirAdimAt();
    _yHiz = _yHiz + _yHareketIvmeAdimBasinaDegisecekHiz;
  }
  // Konum güncelleme.
  _yKonumDerecesi = gidilecekKonumunDerecesi;
}

void SM::_cifteHareket(unsigned int xGidilecekKonumunDerecesi, unsigned int yGidilecekKonumunDerecesi){
  // Yönlerin belirlenmesi.
  if(_xKonumDerecesi < xGidilecekKonumunDerecesi){ // 0dan 100e
    _xYonAyarla(HIGH);
  }else{                                       // 100den 0a
    _xYonAyarla(LOW);
  }
  if(_yKonumDerecesi < yGidilecekKonumunDerecesi){ // 0dan 100e  
    _yYonAyarla(HIGH);
  }else{                                       // 100den 0a
    _yYonAyarla(LOW);
  }
  // Kaç derecelik hareket yapılacağının belirlenmesi.
  _xGidilecekDereceSayisi = abs(_xKonumDerecesi - xGidilecekKonumunDerecesi);
  _yGidilecekDereceSayisi = abs(_yKonumDerecesi - yGidilecekKonumunDerecesi);
  // Atılacak adımların belirlenmesi.
  _xGidilecekAdimSayisi = _xGidilecekDereceSayisi * _xEkseni1DereceIcinAdimSayisi;
  _yGidilecekAdimSayisi = _yGidilecekDereceSayisi * _yEkseni1DereceIcinAdimSayisi;
  //Hızların belirlenmesi.
  if(_xGidilecekDereceSayisi < 8){  // Kısa mesafe hareket 1/32 için minimum 56 adım. 20-20 ivme adım payı.
    _xHareketIvmeAdimPayi = 20;
    _xHareketIvmeAdimBasinaDegisecekHiz = 10;
    _xHiz = _xHizTutucu + 400;
  }else{  // Uzun mesafe hareket 1/32 için minimum 448 adım. 200-200 ivme adım payı.
    _xHareketIvmeAdimPayi = 200;
    _xHareketIvmeAdimBasinaDegisecekHiz = 1;
    _xHiz = _xHizTutucu;
  } 
  if(_yGidilecekDereceSayisi < 8){  // Kısa mesafe hareket 1/32 için minimum 56 adım. 20-20 ivme adım payı.
    _yHareketIvmeAdimPayi = 20;
    _yHareketIvmeAdimBasinaDegisecekHiz = 10;
    _yHiz = _yHizTutucu + 400;
  }else{  // Uzun mesafe hareket 1/32 için minimum 448 adım. 200-200 ivme adım payı.
    _yHareketIvmeAdimPayi = 200;
    _yHareketIvmeAdimBasinaDegisecekHiz = 1;
    _yHiz = _yHizTutucu;
  }
  // Hareket fonksiyonu.
  _xGidilenAdimSayisi = 0;
  _yGidilenAdimSayisi = 0;
  while((_xGidilecekAdimSayisi - _xGidilenAdimSayisi) > 0 || (_yGidilecekAdimSayisi - _yGidilenAdimSayisi) > 0){
    if((_xGidilecekAdimSayisi - _xGidilenAdimSayisi) > 0){
      if(_xGidilenAdimSayisi < _xHareketIvmeAdimPayi){
        _xHiz = _xHiz - _xHareketIvmeAdimBasinaDegisecekHiz;
      }else if(_xGidilecekAdimSayisi - _xGidilenAdimSayisi < _xHareketIvmeAdimPayi){
        _xHiz = _xHiz + _xHareketIvmeAdimBasinaDegisecekHiz;
      }
      _xBirAdimAt();
      _xGidilenAdimSayisi++;
    }else{
      if(_xHiz > 100){
        _xHiz = _xHiz - 2;
      } 
      delayMicroseconds(_xHiz * 2);
    }
    if((_yGidilecekAdimSayisi - _yGidilenAdimSayisi) > 0){
      if(_yGidilenAdimSayisi < _yHareketIvmeAdimPayi){
        _yHiz = _yHiz - _yHareketIvmeAdimBasinaDegisecekHiz;
      }else if((_yGidilecekAdimSayisi - _yGidilenAdimSayisi) < _yHareketIvmeAdimPayi){
        _yHiz = _yHiz + _yHareketIvmeAdimBasinaDegisecekHiz;
      }
      _yBirAdimAt();
      _yGidilenAdimSayisi++;
    }else{
      if(_yHiz > 100){
        _yHiz = _yHiz - 2;  
      }
      delayMicroseconds(_yHiz * 2);
    }
  }
  // Konum güncelleme.
  _xKonumDerecesi = xGidilecekKonumunDerecesi;
  _yKonumDerecesi = yGidilecekKonumunDerecesi;
}

void SM::hizAyarla(unsigned int xHiz, unsigned int yHiz){
  _xHizTutucu = xHiz;
  _yHizTutucu = yHiz;
}

void SM::git(unsigned int xGidilecekKonumunDerecesi, unsigned int yGidilecekKonumunDerecesi){
  if(xGidilecekKonumunDerecesi > 349 || xGidilecekKonumunDerecesi < 0){
    xGidilecekKonumunDerecesi = _xKonumDerecesi;
  }
  if(yGidilecekKonumunDerecesi > 89 || yGidilecekKonumunDerecesi < 0){
    yGidilecekKonumunDerecesi = _yKonumDerecesi;
  }
  if(xGidilecekKonumunDerecesi != _xKonumDerecesi && yGidilecekKonumunDerecesi != _yKonumDerecesi){
    _cifteHareket(xGidilecekKonumunDerecesi, yGidilecekKonumunDerecesi);
  }else if(xGidilecekKonumunDerecesi != _xKonumDerecesi){
    _xTekilHareket(xGidilecekKonumunDerecesi);
  }else if(yGidilecekKonumunDerecesi != _yKonumDerecesi){
    _yTekilHareket(yGidilecekKonumunDerecesi);
  }
}
