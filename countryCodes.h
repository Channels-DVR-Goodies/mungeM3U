//
// Created by paul on 10/3/20.
//

#ifndef MUNGEM3U_COUNTRYCODES_H
#define MUNGEM3U_COUNTRYCODES_H

#define countryHash( a, b )     ( (a  * 43) ^ b)

struct
{
    unsigned int key;
    const char * country;
} kCountryMap = {
    { countryHash( 'A', 'F' ), "Afghanistan" },
    { countryHash( 'A', 'X' ), "Aland Islands" },
    { countryHash( 'A', 'L' ), "Albania" },
    { countryHash( 'D', 'Z' ), "Algeria" },
	{ countryHash( 'A', 'S' ), "American Samoa" },
	{ countryHash( 'A', 'D' ), "Andorra" },
	{ countryHash( 'A', 'O' ), "Angola" },
	{ countryHash( 'A', 'I' ), "Anguilla" },
	{ countryHash( 'A', 'Q' ), "Antarctica" },
	{ countryHash( 'A', 'G' ), "Antigua and Barbuda" },
	{ countryHash( 'A', 'R' ), "Argentina" },
	{ countryHash( 'A', 'M' ), "Armenia" },
	{ countryHash( 'A', 'W' ), "Aruba" },
	{ countryHash( 'A', 'U' ), "Australia" },
	{ countryHash( 'A', 'T' ), "Austria" },
	{ countryHash( 'A', 'Z' ), "Azerbaijan" },
	{ countryHash( 'B', 'S' ), "Bahamas" },
	{ countryHash( 'B', 'H' ), "Bahrain" },
	{ countryHash( 'B', 'D' ), "Bangladesh" },
	{ countryHash( 'B', 'B' ), "Barbados" },
	{ countryHash( 'B', 'Y' ), "Belarus" },
	{ countryHash( 'B', 'E' ), "Belgium" },
	{ countryHash( 'B', 'Z' ), "Belize" },
	{ countryHash( 'B', 'J' ), "Benin" },
	{ countryHash( 'B', 'M' ), "Bermuda" },
	{ countryHash( 'B', 'T' ), "Bhutan" },
	{ countryHash( 'B', 'O' ), "Bolivia, Plurinational State of" },
	{ countryHash( 'B', 'Q' ), "Bonaire, Sint Eustatius and Saba" },
	{ countryHash( 'B', 'A' ), "Bosnia and Herzegovina" },
	{ countryHash( 'B', 'W' ), "Botswana" },
	{ countryHash( 'B', 'V' ), "Bouvet Island" },
	{ countryHash( 'B', 'R' ), "Brazil" },
	{ countryHash( 'I', 'O' ), "British Indian Ocean Territory" },
	{ countryHash( 'B', 'N' ), "Brunei Darussalam" },
	{ countryHash( 'B', 'G' ), "Bulgaria" },
	{ countryHash( 'B', 'F' ), "Burkina Faso" },
	{ countryHash( 'B', 'I' ), "Burundi" },
	{ countryHash( 'K', 'H' ), "Cambodia" },
	{ countryHash( 'C', 'M' ), "Cameroon" },
	{ countryHash( 'C', 'A' ), "Canada" },
	{ countryHash( 'C', 'V' ), "Cape Verde" },
	{ countryHash( 'K', 'Y' ), "Cayman Islands" },
	{ countryHash( 'C', 'F' ), "Central African Republic" },
	{ countryHash( 'T', 'D' ), "Chad" },
	{ countryHash( 'C', 'L' ), "Chile" },
	{ countryHash( 'C', 'N' ), "China" },
	{ countryHash( 'C', 'X' ), "Christmas Island" },
	{ countryHash( 'C', 'C' ), "Cocos (Keeling) Islands" },
	{ countryHash( 'C', 'O' ), "Colombia" },
	{ countryHash( 'K', 'M' ), "Comoros" },
	{ countryHash( 'C', 'G' ), "Congo" },
	{ countryHash( 'C', 'D' ), "Congo, the Democratic Republic of the" },
	{ countryHash( 'C', 'K' ), "Cook Islands" },
	{ countryHash( 'C', 'R' ), "Costa Rica" },
	{ countryHash( 'C', 'I' ), "CÃ´te d'Ivoire" },
	{ countryHash( 'H', 'R' ), "Croatia" },
	{ countryHash( 'C', 'U' ), "Cuba" },
	{ countryHash( 'C', 'W' ), "CuraÃ§ao" },
	{ countryHash( 'C', 'Y' ), "Cyprus" },
	{ countryHash( 'C', 'Z' ), "Czech Republic" },
	{ countryHash( 'D', 'K' ), "Denmark" },
	{ countryHash( 'D', 'J' ), "Djibouti" },
	{ countryHash( 'D', 'M' ), "Dominica" },
	{ countryHash( 'D', 'O' ), "Dominican Republic" },
	{ countryHash( 'E', 'C' ), "Ecuador" },
	{ countryHash( 'E', 'G' ), "Egypt" },
	{ countryHash( 'S', 'V' ), "El Salvador" },
	{ countryHash( 'G', 'Q' ), "Equatorial Guinea" },
	{ countryHash( 'E', 'R' ), "Eritrea" },
	{ countryHash( 'E', 'E' ), "Estonia" },
	{ countryHash( 'E', 'T' ), "Ethiopia" },
	{ countryHash( 'F', 'K' ), "Falkland Islands (Malvinas)" },
	{ countryHash( 'F', 'O' ), "Faroe Islands" },
	{ countryHash( 'F', 'J' ), "Fiji" },
	{ countryHash( 'F', 'I' ), "Finland" },
	{ countryHash( 'F', 'R' ), "France" },
	{ countryHash( 'G', 'F' ), "French Guiana" },
	{ countryHash( 'P', 'F' ), "French Polynesia" },
	{ countryHash( 'T', 'F' ), "French Southern Territories" },
	{ countryHash( 'G', 'A' ), "Gabon" },
	{ countryHash( 'G', 'M' ), "Gambia" },
	{ countryHash( 'G', 'E' ), "Georgia" },
	{ countryHash( 'D', 'E' ), "Germany" },
	{ countryHash( 'G', 'H' ), "Ghana" },
	{ countryHash( 'G', 'I' ), "Gibraltar" },
	{ countryHash( 'G', 'R' ), "Greece" },
	{ countryHash( 'G', 'L' ), "Greenland" },
	{ countryHash( 'G', 'D' ), "Grenada" },
	{ countryHash( 'G', 'P' ), "Guadeloupe" },
	{ countryHash( 'G', 'U' ), "Guam" },
	{ countryHash( 'G', 'T' ), "Guatemala" },
	{ countryHash( 'G', 'G' ), "Guernsey" },
	{ countryHash( 'G', 'N' ), "Guinea" },
	{ countryHash( 'G', 'W' ), "Guinea-Bissau" },
	{ countryHash( 'G', 'Y' ), "Guyana" },
	{ countryHash( 'H', 'T' ), "Haiti" },
	{ countryHash( 'H', 'M' ), "Heard Island and McDonald Islands" },
	{ countryHash( 'V', 'A' ), "Holy See (Vatican City State)" },
	{ countryHash( 'H', 'N' ), "Honduras" },
	{ countryHash( 'H', 'K' ), "Hong Kong" },
	{ countryHash( 'H', 'U' ), "Hungary" },
	{ countryHash( 'I', 'S' ), "Iceland" },
	{ countryHash( 'I', 'N' ), "India" },
	{ countryHash( 'I', 'D' ), "Indonesia" },
	{ countryHash( 'I', 'R' ), "Iran, Islamic Republic of" },
	{ countryHash( 'I', 'Q' ), "Iraq" },
	{ countryHash( 'I', 'E' ), "Ireland" },
	{ countryHash( 'I', 'M' ), "Isle of Man" },
	{ countryHash( 'I', 'L' ), "Israel" },
	{ countryHash( 'I', 'T' ), "Italy" },
	{ countryHash( 'J', 'M' ), "Jamaica" },
	{ countryHash( 'J', 'P' ), "Japan" },
	{ countryHash( 'J', 'E' ), "Jersey" },
	{ countryHash( 'J', 'O' ), "Jordan" },
	{ countryHash( 'K', 'Z' ), "Kazakhstan" },
	{ countryHash( 'K', 'E' ), "Kenya" },
	{ countryHash( 'K', 'I' ), "Kiribati" },
	{ countryHash( 'K', 'P' ), "Korea, Democratic People's Republic of" },
	{ countryHash( 'K', 'R' ), "Korea, Republic of" },
	{ countryHash( 'K', 'W' ), "Kuwait" },
	{ countryHash( 'K', 'G' ), "Kyrgyzstan" },
	{ countryHash( 'L', 'A' ), "Lao People's Democratic Republic" },
	{ countryHash( 'L', 'V' ), "Latvia" },
	{ countryHash( 'L', 'B' ), "Lebanon" },
	{ countryHash( 'L', 'S' ), "Lesotho" },
	{ countryHash( 'L', 'R' ), "Liberia" },
	{ countryHash( 'L', 'Y' ), "Libya" },
	{ countryHash( 'L', 'I' ), "Liechtenstein" },
	{ countryHash( 'L', 'T' ), "Lithuania" },
	{ countryHash( 'L', 'U' ), "Luxembourg" },
	{ countryHash( 'M', 'O' ), "Macao" },
	{ countryHash( 'M', 'K' ), "Macedonia, the Former Yugoslav Republic of" },
	{ countryHash( 'M', 'G' ), "Madagascar" },
	{ countryHash( 'M', 'W' ), "Malawi" },
	{ countryHash( 'M', 'Y' ), "Malaysia" },
	{ countryHash( 'M', 'V' ), "Maldives" },
	{ countryHash( 'M', 'L' ), "Mali" },
	{ countryHash( 'M', 'T' ), "Malta" },
	{ countryHash( 'M', 'H' ), "Marshall Islands" },
	{ countryHash( 'M', 'Q' ), "Martinique" },
	{ countryHash( 'M', 'R' ), "Mauritania" },
	{ countryHash( 'M', 'U' ), "Mauritius" },
	{ countryHash( 'Y', 'T' ), "Mayotte" },
	{ countryHash( 'M', 'X' ), "Mexico" },
	{ countryHash( 'F', 'M' ), "Micronesia, Federated States of" },
	{ countryHash( 'M', 'D' ), "Moldova, Republic of" },
	{ countryHash( 'M', 'C' ), "Monaco" },
	{ countryHash( 'M', 'N' ), "Mongolia" },
	{ countryHash( 'M', 'E' ), "Montenegro" },
	{ countryHash( 'M', 'S' ), "Montserrat" },
	{ countryHash( 'M', 'A' ), "Morocco" },
	{ countryHash( 'M', 'Z' ), "Mozambique" },
	{ countryHash( 'M', 'M' ), "Myanmar" },
	{ countryHash( 'N', 'A' ), "Namibia" },
	{ countryHash( 'N', 'R' ), "Nauru" },
	{ countryHash( 'N', 'P' ), "Nepal" },
	{ countryHash( 'N', 'L' ), "Netherlands" },
	{ countryHash( 'N', 'C' ), "New Caledonia" },
	{ countryHash( 'N', 'Z' ), "New Zealand" },
	{ countryHash( 'N', 'I' ), "Nicaragua" },
	{ countryHash( 'N', 'E' ), "Niger" },
	{ countryHash( 'N', 'G' ), "Nigeria" },
	{ countryHash( 'N', 'U' ), "Niue" },
	{ countryHash( 'N', 'F' ), "Norfolk Island" },
	{ countryHash( 'M', 'P' ), "Northern Mariana Islands" },
	{ countryHash( 'N', 'O' ), "Norway" },
	{ countryHash( 'O', 'M' ), "Oman" },
	{ countryHash( 'P', 'K' ), "Pakistan" },
	{ countryHash( 'P', 'W' ), "Palau" },
	{ countryHash( 'P', 'S' ), "Palestine, State of" },
	{ countryHash( 'P', 'A' ), "Panama" },
	{ countryHash( 'P', 'G' ), "Papua New Guinea" },
	{ countryHash( 'P', 'Y' ), "Paraguay" },
	{ countryHash( 'P', 'E' ), "Peru" },
	{ countryHash( 'P', 'H' ), "Philippines" },
	{ countryHash( 'P', 'N' ), "Pitcairn" },
	{ countryHash( 'P', 'L' ), "Poland" },
	{ countryHash( 'P', 'T' ), "Portugal" },
	{ countryHash( 'P', 'R' ), "Puerto Rico" },
	{ countryHash( 'Q', 'A' ), "Qatar" },
	{ countryHash( 'R', 'E' ), "RÃ©union" },
	{ countryHash( 'R', 'O' ), "Romania" },
	{ countryHash( 'R', 'U' ), "Russian Federation" },
	{ countryHash( 'R', 'W' ), "Rwanda" },
	{ countryHash( 'B', 'L' ), "Saint BarthÃ©lemy" },
	{ countryHash( 'S', 'H' ), "Saint Helena, Ascension and Tristan da Cunha" },
	{ countryHash( 'K', 'N' ), "Saint Kitts and Nevis" },
	{ countryHash( 'L', 'C' ), "Saint Lucia" },
	{ countryHash( 'M', 'F' ), "Saint Martin (French part)" },
	{ countryHash( 'P', 'M' ), "Saint Pierre and Miquelon" },
	{ countryHash( 'V', 'C' ), "Saint Vincent and the Grenadines" },
	{ countryHash( 'W', 'S' ), "Samoa" },
	{ countryHash( 'S', 'M' ), "San Marino" },
	{ countryHash( 'S', 'T' ), "Sao Tome and Principe" },
	{ countryHash( 'S', 'A' ), "Saudi Arabia" },
	{ countryHash( 'S', 'N' ), "Senegal" },
	{ countryHash( 'R', 'S' ), "Serbia" },
	{ countryHash( 'S', 'C' ), "Seychelles" },
	{ countryHash( 'S', 'L' ), "Sierra Leone" },
	{ countryHash( 'S', 'G' ), "Singapore" },
	{ countryHash( 'S', 'X' ), "Sint Maarten (Dutch part)" },
	{ countryHash( 'S', 'K' ), "Slovakia" },
	{ countryHash( 'S', 'I' ), "Slovenia" },
	{ countryHash( 'S', 'B' ), "Solomon Islands" },
	{ countryHash( 'S', 'O' ), "Somalia" },
	{ countryHash( 'Z', 'A' ), "South Africa" },
	{ countryHash( 'G', 'S' ), "South Georgia and the South Sandwich Islands" },
	{ countryHash( 'S', 'S' ), "South Sudan" },
	{ countryHash( 'E', 'S' ), "Spain" },
	{ countryHash( 'L', 'K' ), "Sri Lanka" },
	{ countryHash( 'S', 'D' ), "Sudan" },
	{ countryHash( 'S', 'R' ), "Suriname" },
	{ countryHash( 'S', 'J' ), "Svalbard and Jan Mayen" },
	{ countryHash( 'S', 'Z' ), "Swaziland" },
	{ countryHash( 'S', 'E' ), "Sweden" },
	{ countryHash( 'C', 'H' ), "Switzerland" },
	{ countryHash( 'S', 'Y' ), "Syrian Arab Republic" },
	{ countryHash( 'T', 'W' ), "Taiwan, Province of China" },
	{ countryHash( 'T', 'J' ), "Tajikistan" },
	{ countryHash( 'T', 'Z' ), "Tanzania, United Republic of" },
	{ countryHash( 'T', 'H' ), "Thailand" },
	{ countryHash( 'T', 'L' ), "Timor-Leste" },
	{ countryHash( 'T', 'G' ), "Togo" },
	{ countryHash( 'T', 'K' ), "Tokelau" },
	{ countryHash( 'T', 'O' ), "Tonga" },
	{ countryHash( 'T', 'T' ), "Trinidad and Tobago" },
	{ countryHash( 'T', 'N' ), "Tunisia" },
	{ countryHash( 'T', 'R' ), "Turkey" },
	{ countryHash( 'T', 'M' ), "Turkmenistan" },
	{ countryHash( 'T', 'C' ), "Turks and Caicos Islands" },
	{ countryHash( 'T', 'V' ), "Tuvalu" },
	{ countryHash( 'U', 'G' ), "Uganda" },
	{ countryHash( 'U', 'A' ), "Ukraine" },
	{ countryHash( 'A', 'E' ), "United Arab Emirates" },
	{ countryHash( 'G', 'B' ), "United Kingdom" },
	{ countryHash( 'U', 'S' ), "United States" },
	{ countryHash( 'U', 'M' ), "United States Minor Outlying Islands" },
	{ countryHash( 'U', 'Y' ), "Uruguay" },
	{ countryHash( 'U', 'Z' ), "Uzbekistan" },
	{ countryHash( 'V', 'U' ), "Vanuatu" },
	{ countryHash( 'V', 'E' ), "Venezuela, Bolivarian Republic of" },
	{ countryHash( 'V', 'N' ), "Viet Nam" },
	{ countryHash( 'V', 'G' ), "Virgin Islands, British" },
	{ countryHash( 'V', 'I' ), "Virgin Islands, U.S." },
	{ countryHash( 'W', 'F' ), "Wallis and Futuna" },
	{ countryHash( 'E', 'H' ), "Western Sahara" },
	{ countryHash( 'Y', 'E' ), "Yemen" },
	{ countryHash( 'Z', 'M' ), "Zambia" },
	{ countryHash( 'Z', 'W' ), "Zimbabwe" }
};


#endif //MUNGEM3U_COUNTRYCODES_H
