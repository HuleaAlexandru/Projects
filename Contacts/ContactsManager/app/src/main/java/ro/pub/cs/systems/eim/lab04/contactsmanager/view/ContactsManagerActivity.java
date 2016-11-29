package ro.pub.cs.systems.eim.lab04.contactsmanager.view;

import android.app.Activity;
import android.content.ContentValues;
import android.content.Intent;
import android.os.Bundle;
import android.provider.ContactsContract;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.Toast;

import java.util.ArrayList;

import ro.pub.cs.systems.eim.lab04.contactsmanager.R;
import ro.pub.cs.systems.eim.lab04.contactsmanager.general.Constants;

public class ContactsManagerActivity extends AppCompatActivity {

    private EditText nameEditText = null;
    private EditText phoneNumberEditText = null;
    private EditText emailEditText = null;
    private EditText addressEditText = null;
    private EditText jobTitleEditText = null;
    private EditText companyEditText = null;
    private EditText websiteEditText = null;
    private EditText imEditText = null;

    private Button showHideAdditionalFieldsButton = null;
    private Button saveButton = null;
    private Button cancelButton = null;
    private LinearLayout additionalFieldsContainer = null;

    private ShowHideAdditionalFieldsButtonClickListener showHideAdditionalFieldsButtonClickListener = new ShowHideAdditionalFieldsButtonClickListener();
    private class ShowHideAdditionalFieldsButtonClickListener implements View.OnClickListener {
        @Override
        public void onClick(View view) {
            switch (additionalFieldsContainer.getVisibility()) {
                case View.VISIBLE:
                    additionalFieldsContainer.setVisibility(View.INVISIBLE);
                    showHideAdditionalFieldsButton.setText(getResources().getString(R.string.show_additional_fields));
                    break;
                case View.INVISIBLE:
                    additionalFieldsContainer.setVisibility(View.VISIBLE);
                    showHideAdditionalFieldsButton.setText(getResources().getString(R.string.hide_additional_fields));
                    break;
            }
        }
    }

    private SaveButtonClickListener saveButtonClickListener = new SaveButtonClickListener();
    private class SaveButtonClickListener implements View.OnClickListener {
        @Override
        public void onClick(View view) {
            Intent intent = new Intent(ContactsContract.Intents.Insert.ACTION);
            intent.setType(ContactsContract.RawContacts.CONTENT_TYPE);
            String name = nameEditText.getText().toString();
            String phoneNumber = phoneNumberEditText.getText().toString();
            String email = emailEditText.getText().toString();
            String address = addressEditText.getText().toString();
            String jobTitle = jobTitleEditText.getText().toString();
            String company = companyEditText.getText().toString();
            String website = websiteEditText.getText().toString();
            String im = imEditText.getText().toString();
            if (name != null) {
                intent.putExtra(ContactsContract.Intents.Insert.NAME, name);
            }
            if (phoneNumber != null) {
                intent.putExtra(ContactsContract.Intents.Insert.PHONE, phoneNumber);
            }
            if (email != null) {
                intent.putExtra(ContactsContract.Intents.Insert.EMAIL, email);
            }
            if (address != null) {
                intent.putExtra(ContactsContract.Intents.Insert.POSTAL, address);
            }
            if (jobTitle != null) {
                intent.putExtra(ContactsContract.Intents.Insert.JOB_TITLE, jobTitle);
            }
            if (company != null) {
                intent.putExtra(ContactsContract.Intents.Insert.COMPANY, company);
            }
            ArrayList<ContentValues> contactData = new ArrayList<ContentValues>();
            if (website != null) {
                ContentValues websiteRow = new ContentValues();
                websiteRow.put(ContactsContract.Data.MIMETYPE, ContactsContract.CommonDataKinds.Website.CONTENT_ITEM_TYPE);
                websiteRow.put(ContactsContract.CommonDataKinds.Website.URL, website);
                contactData.add(websiteRow);
            }
            if (im != null) {
                ContentValues imRow = new ContentValues();
                imRow.put(ContactsContract.Data.MIMETYPE, ContactsContract.CommonDataKinds.Im.CONTENT_ITEM_TYPE);
                imRow.put(ContactsContract.CommonDataKinds.Im.DATA, im);
                contactData.add(imRow);
            }
            intent.putParcelableArrayListExtra(ContactsContract.Intents.Insert.DATA, contactData);
            startActivityForResult(intent, Constants.CONTACTS_MANAGER_REQUEST_CODE);
        }
    }

    private CancelButtonClickListener cancelButtonClickListener = new CancelButtonClickListener();
    private class CancelButtonClickListener implements View.OnClickListener {
        @Override
        public void onClick(View view) {
            setResult(Activity.RESULT_CANCELED, new Intent());
            finish();
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_contacts_manager);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        showHideAdditionalFieldsButton = (Button)findViewById(R.id.show_hide_additional_fields);
        showHideAdditionalFieldsButton.setOnClickListener(showHideAdditionalFieldsButtonClickListener);
        saveButton = (Button)findViewById(R.id.save_button);
        saveButton.setOnClickListener(saveButtonClickListener);
        cancelButton = (Button)findViewById(R.id.cancel_button);
        cancelButton.setOnClickListener(cancelButtonClickListener);

        additionalFieldsContainer = (LinearLayout)findViewById(R.id.additional_fields_container);

        nameEditText = (EditText)findViewById(R.id.name_edit_text);
        phoneNumberEditText = (EditText)findViewById(R.id.phone_number_edit_text);
        emailEditText = (EditText)findViewById(R.id.email_edit_text);
        addressEditText = (EditText)findViewById(R.id.address_edit_text);
        jobTitleEditText = (EditText)findViewById(R.id.job_title_edit_text);
        companyEditText = (EditText)findViewById(R.id.company_edit_text);
        websiteEditText = (EditText)findViewById(R.id.website_edit_text);
        imEditText = (EditText)findViewById(R.id.im_edit_text);

        Intent intent = getIntent();
        if (intent != null) {
            String phoneNumber = intent.getStringExtra(Constants.PHONE_NUMBER_KEY);
            if (phoneNumber != null) {
                phoneNumberEditText.setText(phoneNumber);
            } else {
                Toast.makeText(this, getResources().getString(R.string.phone_number_error), Toast.LENGTH_LONG).show();
            }
        }
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent intent) {
        switch (requestCode) {
            case Constants.CONTACTS_MANAGER_REQUEST_CODE:
                setResult(resultCode, new Intent());
                finish();
                break;
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_contacts_manager, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
}
