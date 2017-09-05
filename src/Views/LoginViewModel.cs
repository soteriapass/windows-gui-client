using System;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Windows.Input;

namespace PasswordManager
{
    class LoginViewModel : INotifyPropertyChanged
    {
        #region Variables

        private readonly LoginView _View;
        private readonly Conf _Model;
        private bool _TwoFactorVisibility;
        private DelegateCommand _OKCommand;

        #endregion

        #region Ctor

        public LoginViewModel(LoginView view, bool twoFactor = false)
        {
            _View = view;
            _Model = App.Instance.Conf;
            _TwoFactorVisibility = twoFactor;
            _OKCommand = new DelegateCommand(delegate { _View.DialogResult = true; _View.Close(); }, CanLogin);
        }

        #endregion

        #region Properties

        public Conf Model
        {
            get { return _Model; }
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

        public string TwoFactorAuthToken
        {
            get { return _Model.TwoFactorAuthToken; }
            set
            {
                if(_Model.TwoFactorAuthToken != value)
                {
                    _Model.TwoFactorAuthToken = value;
                    OnPropertyChanged();
                    _OKCommand.OnCanExecuteChanged();
                }
            }
        }

        public bool TwoFactorVisibility
        {
            get { return _TwoFactorVisibility; }
            set
            {
                if (_TwoFactorVisibility != value)
                {
                    _TwoFactorVisibility = value;
                    OnPropertyChanged();
                }
            }
        }

        public ICommand OKCommand
        {
            get { return _OKCommand; }
        }

        #endregion

        #region Methods

        internal void OnPasswordChanged(string password)
        {
            _Model.Password = password;
            _OKCommand.OnCanExecuteChanged();
        }

        internal void OnPreviewKeyUp(Key key)
        {
            if((key == Key.Enter || key == Key.Return) && _OKCommand.CanExecute(null))
            {
                _OKCommand.Execute(null);
            }
        }

        private bool CanLogin()
        {
            return !string.IsNullOrEmpty(Username) && !string.IsNullOrEmpty(_Model.Password) && (!TwoFactorVisibility || !string.IsNullOrEmpty(TwoFactorAuthToken));
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
