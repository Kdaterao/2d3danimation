# Markdown for Qt functions

## QApplication app(argc, argv);

 * QApplication class manages the GUI application's control flow and main settings.

 * must be declared or something of equivalent in the main loop(i think)

## connect(sender_Object, SignalFunction, recieverObject, RecieverFunction  )

 * sender_Object is the object that will create a signal depending on if a specified function within its class("signalFunction") is called

 * recierverObject is the object that will recieve the signal and then call a specified function within its calss("RecieverFunction")

 * note: the compiler cant use overloaded functions so you will need QOverload<>::of() to specify overloade fucntions

## QOverload<>::of(...SpecifiedOverloadedfunction...) is used for resolving which overloaded function to use in a qt project

 * If you pass an overloaded function into another function (like connect()), it is impossible for the compiler
      to specify which overloaded function is being passed in

 * Thus, to solve this problem for qt (which heavily relies on functions being used via signal and slots) created
        QOverload<>::of() which specifies which overloaded fucntion is being used 

