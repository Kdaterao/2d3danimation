# planning md file 


## What is this ?

 * This is a functionality to store and manage images, and it can be done via the program


 ## how will this work ?

  * the images are not going to be viewable (for now) outside the program, we are going to store them as encoded versions of our raw buffers and parse them and store them when we turn on the program 


## are we going to load in all images at the same time?

 * no we are going to seperate images first by "projects" then further divide them by "layer" and "frames"


## ok so is this class going to handle all of this?

* hmmm, so i definitely want image loading and image storage to be done via this class

* we can most likely leave timeline capabilities to another class, but i think this class can definitely just work 
 on making sure images, their metadata(of course timeline metadata too), ect.. is properly loaded in and stored properly as well...


## ok so is this just going to some utility classes? 

*  im thinking a storage manager with a singleton pattern(feel like there is geniunouly never a good reason to have more than one object to load in data)

* this storage mangager can just handle all of the data loading and such 


## how are we handling meta data?

* we are going to store with xml, and we are going to use tinyxml 

* tinyxml, its pretty lightweight, and i dont forsee anything going wrong with it especiialy since this metadata should never be more than a couple hundred lines 

---
---
---

# architectue

* lets have one xml file that is also looked at when loading the program, this can hold all necessary metaata( file path to project xml, and title, ect..) to load in any project 


* each project has an xml, which specifies the file paths and the timeline data for the project


# folder organization

* base folder of storage
```
/storage (holds everything)

/storage/base.xml (holds all abstracted metadata so we cn find projects ect..)

```

* where projects are stored

```
/storage/projects (holds all the projects)
/storage/projects/"project_name" (each project has its own folder )

```

* inside each project folder

```

/"project_name"/project.xml (holds all the info on the given project )

/"project_name"/raw (this is the raw data)

/"project_name"/output (output video like mp4)


```