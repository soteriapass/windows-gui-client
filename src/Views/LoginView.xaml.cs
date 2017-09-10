namespace PasswordManager
{
    /// <summary>
    /// Interaction logic for LoginView.xaml
    /// </summary>
    public partial class LoginView
    {
        #region Variables

        private readonly LoginViewModel _ViewModel;

        #endregion

        #region Ctor

        public LoginView()
            : this(false)
        {
        }
        public LoginView(bool twoFactor)
        {
            _ViewModel = new LoginViewModel(this, twoFactor);
            DataContext = _ViewModel;

            InitializeComponent();

            if (string.IsNullOrEmpty(_ViewModel.Model.Password))
            {
                PasswordBox.Focus();
            }
            else
            {
                PasswordBox.Password = _ViewModel.Model.Password;
                TwoFactorAuthToken.Focus();
            }
        }

        #endregion

        #region Methods

        private void PasswordBox_PasswordChanged(object sender, System.Windows.RoutedEventArgs e)
        {
            System.Windows.Controls.PasswordBox passwordBox = (System.Windows.Controls.PasswordBox)sender;
            _ViewModel.OnPasswordChanged(passwordBox.Password);
        }

        private void LoginView_PreviewKeyUp(object sender, System.Windows.Input.KeyEventArgs e)
        {
            _ViewModel.OnPreviewKeyUp(e.Key);
        }

        #endregion
    }
}
