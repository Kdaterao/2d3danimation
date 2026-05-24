#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include <toonzRaster.h>



class storageManager{


    public:
    //----- variables -----

    //----- methods -----
    static storageManager* instance(); // gets our manager object 



    //for now these are going to really basic and just store images like an array (makes sense tho because it matches up well with image timeline)
    template <class T>
    ToonzRasterPT<T> loadImages();
    void storeImages();



    private:
    //----- variables -----
    static storageManager* m_instance;

    //----- constructor -----
    storageManager() {};


};


#endif