using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace PasswordManager
{
    class LoginViewModel : INotifyPropertyChanged
    {
        #region Variables

        private readonly LoginView _View;
        private readonly Conf _Model;

        #endregion

        #region Ctor

        public LoginViewModel(LoginView view)
        {
            _View = view;
            _Model = App.Instance.Conf;
        }

        #endregion

        #region Properties

        public string Username
        {
            get { return _Model.Username; }
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
