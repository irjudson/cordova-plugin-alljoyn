

namespace Cordova.Extension.Commands
{
    using WPCordovaClassLib.Cordova;
    using WPCordovaClassLib.Cordova.Commands;
    using WPCordovaClassLib.Cordova.JSON;
    
    public class Hello : BaseCommand
    {
        public void greet(string args)
        {
            string name = JsonHelper.Deserialize<string>(args);
            string message = "Hello, " + name;
            PluginResult result = new PluginResult(PluginResult.Status.OK, message);
            DispatchCommandResult(result);
        }
    }
}