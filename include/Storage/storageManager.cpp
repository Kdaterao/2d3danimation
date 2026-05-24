#include <storageManager.h>
#include <tinyxml2.h>




//----- methods -----

storageManager* storageManager::m_instance;

storageManager* storageManager::instance(){

    //create if needed
    if(!m_instance) m_instance = new storageManager();
  
    return m_instance;
}


/*
 base xml: list of project name's with metadata


 project xml: list image layers -> allows us to know which folder paths to look for when getting buffer 


*/
void storageManager::storeImages(){


    //----- store image buffer somewhere(preset folder path obv) -----



    //----- check base xml file if our specified timeline array xml is there(if not found then create one) ------


    //----- look for our image name, if not found add it to list -----

}







