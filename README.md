# Drush Vista
_________________________________________

## Intro

With the Vista module, you can get clean parseable output of your website information. You can parse the site name, core version and all enabled modules with their update status. **This requires you to have the `Update` module enabled**.

## Installation

Move to your drush folder:

    cd ~/.drush

Clone the repository:

    git clone git@github.com:tolgap/drush-vista.git

And clear your drush cache:

    drush cc drush

## Usage

Go to your drush installation folder and the Vista commands should be available

    vista-core-version    Return the site version
    (vista-cv)
    vista-modules         Print a JSON array with module (update) information
    (vista-m)
    vista-name (vista-n)  Return the site name