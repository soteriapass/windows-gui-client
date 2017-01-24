using Grpc.Core;
using Pswmgr;
using System.IO;
using System.Windows.Input;

namespace PasswordManager
{
    class MainViewModel
    {
        #region Variables

        private readonly MainView _View;
        private readonly Conf _Model;

        #endregion

        #region Ctor

        public MainViewModel(MainView view)
        {
            _View = view;
            _Model = App.Instance.Conf;

            Authenticate();
        }

        #endregion

        #region Properties

        public ICommand ShowOptions
        {
            get { return new ShowOptionsCommand(_View, this); }
        }

        public ICommand Exit
        {
            get { return new ExitCommand(); }
        }

        #endregion

        #region Methods

        public async void Authenticate()
        {
            if (!_Model.IsValid())
                return;

            if (!File.Exists(_Model.ServerCertificate))
                return;

            string rootCertificate = File.ReadAllText(_Model.ServerCertificate);

            var creds = new SslCredentials();
            Channel channel = new Channel(_Model.AuthenticationChannel, creds);

            AuthenticationRequest request = new AuthenticationRequest();
            var client = new Pswmgr.Authentication.AuthenticationClient(channel);

            if (string.IsNullOrEmpty(_Model.Username) || string.IsNullOrEmpty(_Model.Password))
            {
                LoginView view = new LoginView()
                {
                    Owner = _View
                };
                if (view.ShowDialog() == true)
                {
                    request.Username = _Model.Username;
                    request.Password = "asdf";// _Model.Password; TODO: FIX THIS
                }
            }

            await client.AuthenticateAsync(request);
        }

        #endregion
    }
}
