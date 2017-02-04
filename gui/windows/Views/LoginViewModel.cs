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

        #endregion

        #region Ctor

        public LoginViewModel(LoginView view, bool twoFactor = false)
        {
            _View = view;
            _Model = App.Instance.Conf;
            _TwoFactorVisibility = twoFactor;
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
            get { return new DelegateCommand(delegate { _View.DialogResult = true; _View.Close(); }); }
        }

        #endregion

        #region Methods

        internal void OnPasswordChanged(string password)
        {
            _Model.Password = password;
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
