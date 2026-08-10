# tooloptionswidget  

- this widget should allow the user to select different options for their selected tool ....


- this means that we should have a differnt view for each tool? --> yes ....


- so what do we need to come up with 

1) a proper way to automatically switch between tool options for this widget based on the tool selected 

2) a systematic/predictable pattern for our widget organization of this widget....


---

### Solution

To solve #1 we should just do a 
```QStackedWidget ``` since it allows us to properly stack UI's that are switchable based on INDEX

This means we can just do a enum + case switch to easily switch between UIs





To solve #2 we should focus on our file structure, to do solve this we should have everything under ``` ToolOptionWidget``` then one cpp + .h file for the main toolOptionWidget which holds our switch(enum + switch) logic + our signal and slots connections which can be connected via our tool selection widget (we will work on this later .......) now each smaller widget will be under a folde within ToolOptionWidget called ```views````
we can then have a folde per view widget inside..




---

### QA

- should our color selection be apart of this....? --> prolly not since many tools can use a color(but on the other hand some dont..)

- i think color selection should be its own especially since we will have different views for that one anyways 


- should our tool toggle be in this widget ? --> No because this is the options widget not the selectio nwidget (yes there is a difference )






