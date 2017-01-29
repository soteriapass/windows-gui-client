namespace PasswordManager
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainView
    {
        #region Variables

        private readonly MainViewModel _ViewModel;

        #endregion

        #region Ctor

        public MainView()
        {
            _ViewModel = new MainViewModel(this);
            DataContext = _ViewModel;

            InitializeComponent();
        }

        #endregion
    }
}
