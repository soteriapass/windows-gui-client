using System;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Windows.Input;

namespace PasswordManager
{
    class NewPasswordViewModel : INotifyPropertyChanged
    {
        #region Variables

        private readonly NewPasswordView _View;
        private readonly Pswmgr.PasswordEntry _Model;
        private readonly DelegateCommand _OKCommand;

        #endregion

        #region Ctor

        public NewPasswordViewModel(NewPasswordView view)
        {
            _View = view;
            _Model = new Pswmgr.PasswordEntry();
            _OKCommand = new DelegateCommand(OnOk, DataCompleted);
        }

        #endregion

        #region Properties

        public Pswmgr.PasswordEntry Model
        {
            get { return _Model; }
        }

        public bool DataCompleted()
        {
            return !string.IsNullOrEmpty(_Model.AccountName) && !string.IsNullOrEmpty(_Model.Extra) && !string.IsNullOrEmpty(_Model.Password) && !string.IsNullOrEmpty(_Model.Username);
        }

        public ICommand OKCommand
        {
            get { return _OKCommand; }
        }

        internal void OnPasswordBoxPasswordChanged()
        {
            _Model.Password = _View._PasswordBox.Password;
        }

        public string AccountName
        {
            get { return _Model.AccountName; }
            set
            {
                if(_Model.AccountName != value)
                {
                    _Model.AccountName = value;
                    OnPropertyChanged();
                    _OKCommand.OnCanExecuteChanged();
                }
            }
        }

        public string Username
        {
            get { return _Model.Username; }
            set
            {
                if(_Model.Username != value)
                {
                    _Model.Username = value;
                    OnPropertyChanged();
                    _OKCommand.OnCanExecuteChanged();
                }
            }
        }

        public string Extra
        {
            get { return _Model.Extra; }
            set
            {
                if(_Model.Extra != value)
                {
                    _Model.Extra = value;
                    OnPropertyChanged();
                    _OKCommand.OnCanExecuteChanged();
                }
            }
        }

        #endregion

        #region Methods

        private void OnOk()
        {
            _View.DialogResult = true;
            _View.Close();
        }

        #endregion

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        protected void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        #endregion
    }
}
