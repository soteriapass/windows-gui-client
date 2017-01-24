namespace PasswordManager
{
    class ShowOptionsCommand : AbstractCommand
    {
        protected override void Execute()
        {
            OptionsView view = new OptionsView();
            view.ShowDialog();
        }
    }
}
