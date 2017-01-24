namespace PasswordManager
{
    /// <summary>
    /// Interaction logic for OptionsView.xaml
    /// </summary>
    public partial class OptionsView
    {
        #region Variable

        private readonly OptionsViewModel _ViewModel;

        #endregion

        #region Ctor

        public OptionsView()
        {
            _ViewModel = new OptionsViewModel();
            DataContext = _ViewModel;

            InitializeComponent();
        }

        #endregion
    }
}
