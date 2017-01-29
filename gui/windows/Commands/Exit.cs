namespace PasswordManager
{
    class ExitCommand : AbstractCommand
    {
        protected override void Execute()
        {
            App.Current.Shutdown();
        }
    }
}
