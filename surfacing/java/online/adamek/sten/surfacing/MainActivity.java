package online.adamek.sten.surfacing;

import android.graphics.SurfaceTexture;
import android.support.annotation.IntRange;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.TextureView;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.TextView;

import java.util.Locale;


public class MainActivity
        extends AppCompatActivity
        implements AdapterView.OnItemSelectedListener,
                  TextureView.SurfaceTextureListener
{
    static
    {
        System.loadLibrary("native");
    }


    /** Spinner for implementation selection. */
    private Spinner mSpinner;

    /** Texture for drawing to. */
    private TextureView mTextureView;

    /** Log text views. */
    private TextView mLog[] = new TextView[5];



    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mSpinner = findViewById(R.id.spinner);
        final ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this, R.array.types,
                                                                                   android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpinner.setAdapter(adapter);
        mSpinner.setOnItemSelectedListener(this);

        findViewById(R.id.draw).setOnClickListener(this::onDraw);

        mTextureView = findViewById(R.id.texture);
        mTextureView.setSurfaceTextureListener(this);
        mTextureView.setOpaque(false);

        mLog[0] = findViewById(R.id.log0);
        mLog[1] = findViewById(R.id.log1);
        mLog[2] = findViewById(R.id.log2);
        mLog[3] = findViewById(R.id.log3);
        mLog[4] = findViewById(R.id.log4);
    }



    @Override
    public void onItemSelected(@NonNull AdapterView<?> parent,
                               @NonNull View view,
                               int position,
                               long id)
    {
        onSelected(position);
    }

    @Override
    public void onNothingSelected(@NonNull AdapterView<?> parent)
    {
        onSelected(0);
    }

    public void onSelected(@IntRange(from=0) int selection)
    {
        final SurfaceTexture texture = mTextureView.getSurfaceTexture();
        if (texture != null)
            onSelected(texture, selection);
    }

    public native void onSelected(@Nullable SurfaceTexture texture,
                                  @IntRange(from=0) int selection);



    public void onDraw(@NonNull View view)
    {
        onDraw();
    }

    public native void onDraw();



    @Override
    public void onSurfaceTextureAvailable(@NonNull SurfaceTexture surface,
                                          @IntRange(from=1) int width,
                                          @IntRange(from=1) int height)
    {
        log(String.format(Locale.ROOT, "Texture created (%d×%d)", width, height));
        final int position = mSpinner.getSelectedItemPosition();
        if (position >= 0)
            onSelected(position);
    }


    @Override
    public void onSurfaceTextureSizeChanged(@NonNull SurfaceTexture surface,
                                            @IntRange(from=1) int width,
                                            @IntRange(from=1) int height)

    {
        log(String.format(Locale.ROOT, "Texture resized (%d×%d)", width, height));
        /* The surface remains valid so no reinitialization is needed but its dimensions have
         * changed. You may want to recalculate OpenGL matrix or update buffer geometry here. */
    }


    @Override
    public boolean onSurfaceTextureDestroyed(@NonNull SurfaceTexture surface)
    {
        log("Texture destroyed");
        // Deinitialize
        onSelected(null, 0);
        return true;
    }


    @Override
    public void onSurfaceTextureUpdated(@NonNull SurfaceTexture surface)
    {
        // Each draw updates the texture and logs on its own
    }


    private void log(@NonNull String message)
    {
        mLog[4].setText(mLog[3].getText());
        mLog[3].setText(mLog[2].getText());
        mLog[2].setText(mLog[1].getText());
        mLog[1].setText(mLog[0].getText());
        mLog[0].setText(message);
    }
}
