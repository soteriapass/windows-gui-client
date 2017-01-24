using System;
using System.Windows.Input;

namespace PasswordManager
{
    class DelegateCommand : ICommand
    {
        #region Variables

        private readonly Action _Action;

        #endregion

        #region Ctor

        public DelegateCommand(Action action)
        {
            _Action = action;
        }

        #endregion

        #region Methods

        public bool CanExecute(object parameter)
        {
            return true;
        }

        public void Execute(object parameter)
        {
            _Action();
        }

        #endregion

        #region Events

        public event EventHandler CanExecuteChanged;

        protected void OnCanExecuteChanged()
        {
            CanExecuteChanged?.Invoke(this, EventArgs.Empty);
        }

        #endregion
    }
}
