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
        {
            _ViewModel = new LoginViewModel(this);
            DataContext = _ViewModel;

            InitializeComponent();
        }

        #endregion

        private void PasswordBox_PasswordChanged(object sender, System.Windows.RoutedEventArgs e)
        {

        }
    }
}
