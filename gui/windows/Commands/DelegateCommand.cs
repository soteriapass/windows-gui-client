using System;
using System.Windows.Input;

namespace PasswordManager
{
    class DelegateCommand : ICommand
    {
        #region Constants

        public delegate bool CanExecuteDelegate();

        #endregion

        #region Variables

        private readonly Action _Action;
        private readonly CanExecuteDelegate _CanExecute;

        #endregion

        #region Ctor

        public DelegateCommand(Action action, CanExecuteDelegate canExecute = null)
        {
            _Action = action;
            _CanExecute = canExecute ?? CanExecuteDefaultImpl;
        }

        #endregion

        #region Methods

        public bool CanExecute(object parameter)
        {
            return _CanExecute();
        }

        private bool CanExecuteDefaultImpl()
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

        public void OnCanExecuteChanged()
        {
            CanExecuteChanged?.Invoke(this, EventArgs.Empty);
        }

        #endregion
    }
}
